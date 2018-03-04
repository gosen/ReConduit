#ifndef __MUX_CONDUIT__HPP__
#define __MUX_CONDUIT__HPP__

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
    constexpr auto insertInSideB(auto&& key, Conduit& b)
    {
        Conduit* next;
        auto insert = [ & ](auto&& mux_ptr) { next = mux_ptr->insert(key, b); };
        dispatch(mux_, insert);
        return next;
    }

    constexpr auto eraseFromSideB(auto&& key)
    {
        Conduit* erased;
        auto erase = [ & ](auto&& mux_ptr) { erased = mux_ptr->erase( key ); };
        dispatch(mux_, erase);
        return erased;
    }

    constexpr auto accept(auto&& v_msg, Conduit* ctx_conduit)
    {
        SPDLOG_DEBUG(getLogger(), "Mux Conduit [{:p}] with [a,b0] -> [{:p}, {:p}] accepts a new message.",
                static_cast<void*>(this), static_cast<void*>(conduit_to_side_a_), static_cast<void*>(conduit_to_side_b0_));
        NextSide next;
        auto accept = [ & ](auto&& mux_ptr, auto&& msg_ptr) { next = mux_ptr->accept( *msg_ptr ); };
        doubleDispatch(mux_, v_msg, accept);
        switch( next ) {
            case NextSide::a:  return conduit_to_side_a_;
            case NextSide::b0: return conduit_to_side_b0_;
            default: return accept_default(v_msg, ctx_conduit);
        }
    }

    constexpr auto accept_default(auto&& v_msg, Conduit* ctx_conduit)
    {
        Conduit* next_conduit;
        bool found;
        auto find = [ & ](auto&& mux_ptr, auto&& msg_ptr)
        {
            auto [nx, f] = mux_ptr->find( *msg_ptr );
            next_conduit = nx; found = f;
        };
        doubleDispatch(mux_, v_msg, find);
        if( ! found ) {
            SPDLOG_DEBUG(getLogger(), "Mux Conduit [{:p}] with [a,b0] -> [{:p}, {:p}] needs new route to be created.",
                static_cast<void*>(this), static_cast<void*>(conduit_to_side_a_), static_cast<void*>(conduit_to_side_b0_));
            auto create = [ & ](auto&& mux_ptr, auto&& msg_ptr)
            {
                auto setup_msg = mux_ptr->create(ctx_conduit, *msg_ptr);
                this->conduit_to_side_b0_->accept( setup_msg );
                next_conduit = nullptr;
            };
            doubleDispatch(mux_, v_msg, create);
        }
        SPDLOG_DEBUG(getLogger(), "Mux Conduit [{:p}] with [a,b] -> [{:p}, {:p}] is routing this message.",
                static_cast<void*>(this), static_cast<void*>(conduit_to_side_a_), static_cast<void*>(next_conduit));
        return next_conduit;
    }

    Conduit* conduit_to_side_a_;
    Conduit* conduit_to_side_b0_;
    mux_type mux_;
};

}

#endif  // __MUX_CONDUIT__HPP__
