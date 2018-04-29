#ifndef __RECONDUIT_DISPACHER__HPP__
#define __RECONDUIT_DISPACHER__HPP__

#include "ReConduitFwd.hpp"
#include "ReConduitVisitors.hpp"
#include "ReConduitLogger.hpp"
#include "ReConduitPool.hpp"

#include <type_traits>

namespace reconduits {

class Conduit
{
public:

    Conduit(const Conduit&)            = delete;
    Conduit& operator=(const Conduit&) = delete;
    Conduit(Conduit&&)                 = delete;
    Conduit& operator=(Conduit&&)      = delete;

    template<typename T, typename = std::enable_if_t< ! std::is_same_v<std::decay_t<T>, Conduit> > >
    explicit Conduit(T&& c)
        : conduit_{ new ( getFromPool<sizeof(T)>() ) T{ std::forward<T>( c ) } }
    {
        SPDLOG_DEBUG(getLogger(), "New Conduit [{0:p}] created", static_cast<void*>(this));
    }

    ~Conduit()
    {
        auto delete_conduit = [ & ](auto&& conduit_ptr)
        {
            SPDLOG_DEBUG(getLogger(), "Deleting Conduit [{0:p}] with variant pointer [{1:p}]", static_cast<void*>(this), static_cast<void*>(conduit_ptr));
            using T = std::decay_t<decltype(*conduit_ptr)>;
            conduit_ptr->~T();
            putToPool<sizeof(T)>( conduit_ptr );
        };
        dispatch(conduit_, delete_conduit);
    }

    void setSideA(Conduit& a) noexcept
    {
        auto set_a = [ &a ](auto&& conduit_ptr) { conduit_ptr->setSideA( a ); };
        dispatch(conduit_, set_a);
    }
    void setSideB(Conduit& b) noexcept
    {
        auto set_b = [ &b ](auto&& conduit_ptr) { conduit_ptr->setSideB( b ); };
        dispatch(conduit_, set_b);
    }

    constexpr Conduit* insertInSideB(auto&& key, Conduit& b)
    {
        auto insert_b = [ & ](auto&& conduit_ptr) { return conduit_ptr->insertInSideB(key, b); };
        return dispatchFor_r<Conduit*, Mux*>(conduit_, insert_b);
    }

    constexpr Conduit* eraseFromSideB(auto&& key)
    {
        auto erase_b = [ & ](auto&& conduit_ptr) { return conduit_ptr->eraseFromSideB( key ); };
        return dispatchFor_r<Conduit*, Mux*>(conduit_, erase_b);
    }

    constexpr void accept(auto&& msg)
    {
        auto v_msg = make_variant_message( msg );
        auto accept = [ & ](auto&& conduit_ptr) { return conduit_ptr->accept(v_msg, this); };
        auto [ next_conduit, next_v_msg ] = dispatch_r(conduit_, accept);
        if( next_conduit ) {
            auto move_next = [ & ](auto&& message_ptr) { next_conduit->accept( *message_ptr ); };
            dispatch(next_v_msg, move_next);
        }
    }

private:

    conduit_type conduit_;
};

}

#endif  // __RECONDUIT_DISPACHER__HPP__
