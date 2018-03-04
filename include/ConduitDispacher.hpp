#ifndef __CONDUIT_DISPACHER__HPP__
#define __CONDUIT_DISPACHER__HPP__

#include "ConduitFwd.hpp"
#include "ConduitVisitors.hpp"
#include "ConduitLogger.hpp"
#include "ConduitPool.hpp"

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

    constexpr bool insertInSideB(auto&& key, Conduit& b)
    {
        Conduit* next;
        auto insert_b = [ & ](auto&& conduit_ptr) { next = conduit_ptr->insertInSideB(key, b); };
        dispatchFor<Mux*>(conduit_, insert_b);
        return next;
    }

    constexpr Conduit* eraseFromSideB(auto&& key)
    {
        Conduit* next;
        auto erase_b = [ & ](auto&& conduit_ptr) { next = conduit_ptr->eraseFromSideB( key ); };
        dispatchFor<Mux*>(conduit_, erase_b);
        return next;
    }

    constexpr void accept(auto&& msg)
    {
        auto v_msg = message_type<embedded_t<decltype(msg)>>{ &msg };
        Conduit* next;
        auto accept = [ & ](auto&& conduit_ptr) { next = conduit_ptr->accept(v_msg, this); };
        dispatch(conduit_, accept);
        if( next ) next->accept( msg );
    }

private:

    conduit_type conduit_;
};

}

#endif  // __CONDUIT_DISPACHER__HPP__
