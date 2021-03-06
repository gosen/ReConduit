#ifndef __MUX_RECONDUIT__HPP__
#define __MUX_RECONDUIT__HPP__

#include "ReConduitVisitors.hpp"
#include "ReConduitPool.hpp"
#include "ReConduitFwd.hpp"

#include <type_traits>

namespace reconduits {

class Mux
{
public:

    template<typename T, typename = std::enable_if_t< ! std::is_same_v<std::decay_t<T>, Mux> > >
    explicit Mux(T&& m)
        : conduit_to_side_a_{}
        , conduit_to_side_b0_{}
        , mux_{ new ( getFromPool<sizeof(T)>() ) T{ std::forward<T>( m ) } }
    {
        SPDLOG_DEBUG(getLogger(), "New Mux Conduit [{0:p}] created", static_cast<void*>(this));
    }

    Mux(Mux&& rhs)
        : conduit_to_side_a_{ std::move( conduit_to_side_a_ ) }
        , conduit_to_side_b0_{ std::move( conduit_to_side_b0_ ) }
        , mux_{ std::move( rhs.mux_ ) }
    {
        SPDLOG_DEBUG(getLogger(), "Mux Conduit [{0:p}] moved", static_cast<void*>(this));
        rhs.conduit_to_side_a_ = rhs.conduit_to_side_b0_ = nullptr;
        auto move_mux = [ &rhs ](auto&& mux_ptr)
        {
            rhs.mux_ = static_cast<std::decay_t<decltype(mux_ptr)>>( nullptr );
        };
        dispatch(rhs.mux_, move_mux);
    }

    ~Mux()
    {
        auto delete_mux = [ & ](auto&& mux_ptr)
        {
            SPDLOG_DEBUG(getLogger(), "Deleting Mux Conduit [{0:p}] with variant pointer [{1:p}]", static_cast<void*>(this), static_cast<void*>(mux_ptr));
            if( mux_ptr ) {
                using T = std::decay_t<decltype( *mux_ptr )>;
                mux_ptr->~T();
                putToPool<sizeof(T)>( mux_ptr );
            }
        };
        dispatch(mux_, delete_mux);
    }

    Mux(const Mux& rhs)        = delete;
    Mux& operator=(const Mux&) = delete;
    Mux& operator=(Mux&&)      = delete;

private:

    friend Conduit;

    constexpr void setSideA(Conduit&  a) noexcept { conduit_to_side_a_  = &a; }
    constexpr void setSideB(Conduit& b0) noexcept { conduit_to_side_b0_ = &b0; }
    constexpr Conduit* insertInSideB(auto&& key, Conduit& b)
    {
        auto insert = [ & ](auto&& mux_ptr) { return mux_ptr->insert(key, b); };
        return dispatch_r(mux_, insert);
    }

    constexpr Conduit* eraseFromSideB(auto&& key)
    {
        auto erase = [ & ](auto&& mux_ptr) { return mux_ptr->erase( key ); };
        return dispatch_r(mux_, erase);
    }

    constexpr auto accept(auto&& v_msg, Conduit* ctx_conduit)
    {
        SPDLOG_DEBUG(getLogger(), "Mux Conduit [{:p}] with [a,b0] -> [{:p}, {:p}] accepts a new message.",
                static_cast<void*>(this), static_cast<void*>(conduit_to_side_a_), static_cast<void*>(conduit_to_side_b0_));
        auto accept = [ &ctx_conduit ](auto&& mux_ptr, auto&& msg_ptr) { return mux_ptr->accept(*msg_ptr, ctx_conduit); };
        auto [ next, next_v_msg ] = doubleDispatch_r(mux_, v_msg, accept);
        switch( next ) {
            case NextSide::a:  return std::pair{ conduit_to_side_a_, next_v_msg };
            case NextSide::b0: return std::pair{ conduit_to_side_b0_, next_v_msg };
            default:           return selectConduitSideB(v_msg, ctx_conduit);
        }
    }

    constexpr auto selectConduitSideB(auto&& v_msg, Conduit* ctx_conduit)
    {
        auto find = [](auto&& mux_ptr, auto&& msg_ptr) { return mux_ptr->find( *msg_ptr ); };
        if( auto [next_conduit, found] = doubleDispatch_r(mux_, v_msg, find); found ) {
            SPDLOG_DEBUG(getLogger(), "Mux Conduit [{:p}] with [a,b] -> [{:p}, {:p}] is routing this message.",
                    static_cast<void*>(this), static_cast<void*>(conduit_to_side_a_), static_cast<void*>(next_conduit));
            return std::pair{ next_conduit, v_msg };
        } else {
            SPDLOG_DEBUG(getLogger(), "Mux Conduit [{:p}] with [a,b0] -> [{:p}, {:p}] needs new route to be created.",
                static_cast<void*>(this), static_cast<void*>(conduit_to_side_a_), static_cast<void*>(conduit_to_side_b0_));
            auto setup = [ & ](auto&& mux_ptr, auto&& msg_ptr) { return mux_ptr->setup(*msg_ptr, ctx_conduit); };
            auto setup_v_msg = doubleDispatch_r(mux_, v_msg, setup);
            return std::pair{ conduit_to_side_b0_, setup_v_msg };
        }
    }

    Conduit* conduit_to_side_a_;
    Conduit* conduit_to_side_b0_;
    mux_type mux_;
};

}

#endif  // __MUX_RECONDUIT__HPP__
