#ifndef __ADAPTER_CONDUIT__HPP__
#define __ADAPTER_CONDUIT__HPP__

#include "ReConduitVisitors.hpp"
#include "ReConduitPool.hpp"

#include <type_traits>

namespace reconduits {

class Conduit;

class Adapter
{
public:

    template<typename T, typename = std::enable_if_t< ! std::is_same_v<std::decay_t<T>, Adapter> > >
    Adapter(T&& a)
        : conduit_to_side_a_{}
        , adapter_{ new ( getFromPool<sizeof(T)>() ) T{ std::forward<T>( a ) } }
    {
        SPDLOG_DEBUG(getLogger(), "New Adapter Conduit [{0:p}] created", static_cast<void*>(this));
    }

    Adapter(Adapter&& rhs)
        : conduit_to_side_a_{ std::move( rhs.conduit_to_side_a_ ) }
        , adapter_{ std::move( rhs.adapter_ ) }
    {
        SPDLOG_DEBUG(getLogger(), "Adapter Conduit [{0:p}] moved", static_cast<void*>(this));
        rhs.conduit_to_side_a_ = nullptr;
        auto move_adapter = [ &rhs ](auto&& adapter_ptr)
        {
            rhs.adapter_ = static_cast<std::decay_t<decltype(adapter_ptr)>>( nullptr );
        };
        dispatch(rhs.adapter_, move_adapter);
    }

    ~Adapter()
    {
        auto delete_adapter = [ & ](auto&& adapter_ptr)
        {
            SPDLOG_DEBUG(getLogger(), "Deleting Adapter Conduit [{0:p}] with variant pointer [{1:p}]", static_cast<void*>(this), static_cast<void*>(adapter_ptr));
            if( adapter_ptr ) {
                using T = std::decay_t<decltype( *adapter_ptr )>;
                adapter_ptr->~T();
                putToPool<sizeof(T)>( adapter_ptr );
            }
        };
        dispatch(adapter_, delete_adapter);
    }

    Adapter(const Adapter& rhs)        = delete;
    Adapter& operator=(const Adapter&) = delete;
    Adapter& operator=(Adapter&&)      = delete;

private:

    friend Conduit;

    constexpr void setSideA(Conduit& a) noexcept { conduit_to_side_a_ = &a; }
    constexpr void setSideB(Conduit&  ) noexcept {}

    constexpr auto accept(auto&& v_msg, Conduit* ctx_conduit)
    {
        SPDLOG_DEBUG(getLogger(), "Adapter Conduit [{:p}] with [a] -> [{:p}] accepts a new message.",
                static_cast<void*>(this), static_cast<void*>(conduit_to_side_a_));
        NextSide next;
        auto accept = [ & ](auto&& adapter_ptr, auto&& msg_ptr) { next = adapter_ptr->accept( *msg_ptr ); };
        doubleDispatch(adapter_, v_msg, accept);
        return next == NextSide::a ? conduit_to_side_a_ : nullptr;
    }

    Conduit* conduit_to_side_a_;
    adapter_type adapter_;
};

}

#endif  // __ADAPTER_CONDUIT__HPP__
