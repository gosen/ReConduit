#ifndef __PROTOCOL_RECONDUIT__HPP__
#define __PROTOCOL_RECONDUIT__HPP__

#include "ReConduitVisitors.hpp"
#include "ReConduitPool.hpp"

#include <type_traits>

namespace reconduits {

class Conduit;

class Protocol
{
public:

    template<typename T, typename = std::enable_if_t< ! std::is_same_v<std::decay_t<T>, Protocol> > >
    explicit Protocol(T&& p)
        : conduit_to_side_a_{}
        , conduit_to_side_b_{}
        , protocol_{ new ( getFromPool<sizeof(T)>() ) T{ std::forward<T>( p ) } }
    {
        SPDLOG_DEBUG(getLogger(), "New Protocol Conduit [{0:p}] created", static_cast<void*>(this));
    }

    Protocol(Protocol&& rhs)
        : conduit_to_side_a_{ std::move( rhs.conduit_to_side_a_ ) }
        , conduit_to_side_b_{ std::move( rhs.conduit_to_side_b_ ) }
        , protocol_{ std::move( rhs.protocol_ ) }
    {
        SPDLOG_DEBUG(getLogger(), "Protocol Conduit [{0:p}] moved", static_cast<void*>(this));
        rhs.conduit_to_side_a_ = rhs.conduit_to_side_b_ = nullptr;
        auto move_protocol = [ &rhs ](auto&& protocol_ptr)
        {
            rhs.protocol_ = static_cast<std::decay_t<decltype(protocol_ptr)>>( nullptr );
        };
        dispatch(rhs.protocol_, move_protocol);
    }

    ~Protocol()
    {
        auto delete_protocol = [ & ](auto&& protocol_ptr)
        {
            SPDLOG_DEBUG(getLogger(), "Deleting Protocol Conduit [{0:p}] with variant pointer [{1:p}]",
                    static_cast<void*>(this), static_cast<void*>(protocol_ptr));
            if( protocol_ptr ) {
                using T = std::decay_t<decltype(*protocol_ptr)>;
                protocol_ptr->~T();
                putToPool<sizeof(T)>( protocol_ptr );
            }
        };
        dispatch(protocol_, delete_protocol);
    }

    Protocol(const Protocol& rhs)        = delete;
    Protocol& operator=(const Protocol&) = delete;
    Protocol& operator=(Protocol&&)      = delete;

private:

    friend Conduit;

    constexpr void setSideA(Conduit& a) noexcept { conduit_to_side_a_ = &a; }
    constexpr void setSideB(Conduit& b) noexcept { conduit_to_side_b_ = &b; }

    constexpr auto accept(auto&& v_msg, Conduit* ctx_conduit)
    {
        SPDLOG_DEBUG(getLogger(), "Protocol Conduit [{:p}] with [a,b] -> [{:p}, {:p}] accepts a new message.",
                static_cast<void*>(this), static_cast<void*>(conduit_to_side_a_), static_cast<void*>(conduit_to_side_b_));
        NextSide next;
        auto accept = [ & ](auto&& protocol_ptr, auto&& msg_ptr)
        {
            auto [ nx_side, nx_msg ] = protocol_ptr->accept( *msg_ptr );
            next = nx_side; v_msg = nx_msg;
        };
        doubleDispatch(protocol_, v_msg, accept);

        switch( next )
        {
            case NextSide::a: return conduit_to_side_a_;
            case NextSide::b: return conduit_to_side_b_;
            default:          return static_cast<Conduit*>( nullptr );
        }
    }

    Conduit* conduit_to_side_a_;
    Conduit* conduit_to_side_b_;
    protocol_type protocol_;
};

}

#endif  // __PROTOCOL_RECONDUIT__HPP__
