#ifndef __FACTORY_CONDUIT__HPP__
#define __FACTORY_CONDUIT__HPP__

#include "ReConduitVisitors.hpp"
#include "ReConduitPool.hpp"

#include <type_traits>

namespace reconduits {

class Factory
{
public:

    template<typename T, typename = std::enable_if_t< ! std::is_same_v<std::decay_t<T>, Factory> > >
    explicit Factory(T&& f)
        : conduit_to_side_a_{}
        , conduit_to_side_b_{}
        , factory_{ new ( getFromPool<sizeof(T)>() ) T{ std::forward<T>( f ) } }
    {
        SPDLOG_DEBUG(getLogger(), "New Factory Conduit [{0:p}] created", static_cast<void*>(this));
    }

    Factory(Factory&& rhs)
        : conduit_to_side_a_{ std::move( rhs.conduit_to_side_a_ ) }
        , conduit_to_side_b_{ std::move( rhs.conduit_to_side_b_ ) }
        , factory_{ std::move( rhs.factory_ ) }
    {
        SPDLOG_DEBUG(getLogger(), "Factory Conduit [{0:p}] moved", static_cast<void*>(this));
        rhs.conduit_to_side_a_ = rhs.conduit_to_side_b_ = nullptr;
        auto move_factory = [ &rhs ](auto&& factory_ptr)
        {
            rhs.factory_ = static_cast<std::decay_t<decltype(factory_ptr)>>( nullptr );
        };
        dispatch(rhs.factory_, move_factory);
    }

    ~Factory()
    {
        auto delete_factory = [ & ](auto&& factory_ptr)
        {
            SPDLOG_DEBUG(getLogger(), "Deleting Factory Conduit [{0:p}] with variant pointer [{1:p}]", static_cast<void*>(this), static_cast<void*>(factory_ptr));
            if( factory_ptr ) {
                using T = std::decay_t<decltype( *factory_ptr )>;
                factory_ptr->~T();
                putToPool<sizeof(T)>( factory_ptr );
            }
        };
        dispatch(factory_, delete_factory);
    }

    Factory(const Factory& rhs)        = delete;
    Factory& operator=(const Factory&) = delete;
    Factory& operator=(Factory&&)      = delete;

private:

    friend Conduit;

    constexpr void setSideA(Conduit& a) noexcept { conduit_to_side_a_ = &a; }
    constexpr void setSideB(Conduit& b) noexcept { conduit_to_side_b_ = &b; }

    constexpr auto accept(auto&& v_msg, Conduit* ctx_conduit)
    {
        SPDLOG_DEBUG(getLogger(), "Factory Conduit [{:p}] with [a,b] -> [{:p}, {:p}] accepts a new message.",
                static_cast<void*>(this), static_cast<void*>(conduit_to_side_a_), static_cast<void*>(conduit_to_side_b_));
        Conduit* next;
        auto accept = [ & ](auto&& factory_ptr, auto&& msg_ptr)
        {
            next = factory_ptr->accept(*msg_ptr, this->conduit_to_side_a_, this->conduit_to_side_b_);
        };
        doubleDispatch(factory_, v_msg, accept);
        return next;
    }

    Conduit* conduit_to_side_a_;
    Conduit* conduit_to_side_b_;
    factory_type factory_;
};

}

#endif  // __FACTORY_CONDUIT__HPP__
