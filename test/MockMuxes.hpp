#pragma once

#include "ReConduitTypesGenerators.hpp"
#include "MockLogger.hpp"
#include "MockMessage.hpp"
#include "MockTCPStateMachine.hpp"

#include "sol/sol.hpp"

#include <unordered_map>
#include <utility>
#include <type_traits>
#include <string>
#include <sstream>

namespace mock_conduits {

//////////////////////////////////////
// Muxes
//////////////////////////////////////

class L3Mux
{
public:

    using key_type   = typename Message::key_type;
    using value_type = reconduits::Conduit*;

    constexpr auto accept(auto&& msg, reconduits::Conduit*)
    {
        using namespace reconduits;
        SPDLOG_DEBUG(getLogger(), "L3Mux [{:p}] accepts a new message.", static_cast<void*>(this));
        auto& emsg = msg.get();
        emsg.append( "L3Mux" );
        return std::pair{ NextSide::b, make_variant_message( msg ) };
    }

    auto find(auto&& msg) const
    {
        switch( std::get<mock_packet::ProtocolType>( msg.get().getL3Id() ) ) {
            case mock_packet::ProtocolType::tcp:
                return std::pair{ tcp_protocol_, tcp_protocol_ != nullptr };
            case mock_packet::ProtocolType::udp:
                return std::pair{ udp_protocol_, udp_protocol_ != nullptr };
            default:
                return std::pair{ static_cast<reconduits::Conduit*>( nullptr ), false };
        }
    }

    auto insert(auto&& key, reconduits::Conduit& c)
    {
        switch( std::get<mock_packet::ProtocolType>( key ) ) {
            case mock_packet::ProtocolType::tcp:
                return tcp_protocol_ = &c;
            case mock_packet::ProtocolType::udp:
                return udp_protocol_ = &c;
            default:
                return static_cast<reconduits::Conduit*>( nullptr );
        }
    }

    auto setup(auto&& msg, reconduits::Conduit* conduit_origin) const
    {
        using namespace reconduits;
        SPDLOG_DEBUG(getLogger(), "L3Mux [{:p}] is going to create new conduits.", static_cast<const void*>(this));
        return make_variant_setup_message(msg, conduit_origin);
    }

    auto erase(auto&& key)
    {
        return static_cast<reconduits::Conduit*>( nullptr );
    }

protected:

    value_type tcp_protocol_;
    value_type udp_protocol_;
};

class L4Mux
{
public:

    using key_type   = typename mock_packet::Packet::l4_id_type;
    using value_type = reconduits::Conduit*;

    constexpr auto accept(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        auto& emsg = msg.get();
        SPDLOG_DEBUG(getLogger(), "L4Mux [{:p}] accepts a new message.", static_cast<void*>(this));
        emsg.append( "L4Mux" );

        return accept_(msg, conduit_origin);
    }

    auto find(auto&& msg) const
    {
        //SPDLOG_DEBUG(getLogger(), "L4Mux [{0:p}] has{1}found key {2}.", static_cast<const void*>(this), (found ? " ":" NOT "),
        //        std::get<mock_packet::Packet::l4_id_type>( msg.get().getL4Id() ));
        return ctx_ptr_ ? std::pair{ctx_ptr_->next_conduit_, true} : std::pair{nullptr, false};
    }

    auto insert(auto&& key, reconduits::Conduit& c)
    {
        SPDLOG_DEBUG(getLogger(), "L4Mux [{:p}] is going to connect new conduits.", static_cast<void*>(this));
        if( ctx_ptr_ ) {
            ctx_ptr_->next_conduit_ = &c;
            return &c;
        } else {
            //getLogger()->warn("The new connection for key {} was not possible.", std::get<mock_packet::Packet::l4_id_type>( key ));
            return static_cast<reconduits::Conduit*>( nullptr );
        }
    }

    auto setup(auto&& msg, reconduits::Conduit* conduit_origin) const
    {
        using namespace reconduits;
        SPDLOG_DEBUG(getLogger(), "L4Mux [{:p}] is going to create new conduits.", static_cast<const void*>(this));
        return make_variant_setup_message(msg, conduit_origin);
    }

    auto erase(auto&& key)
    {
        SPDLOG_DEBUG(getLogger(), "L4Mux [{:p}] is going to release connected conduits.", static_cast<void*>(this));
        auto it = mux_table_.find( std::get<mock_packet::Packet::l4_id_type>( key ) );
        if( it != mux_table_.cend() ) {
            mux_table_.erase( it );
            return it->second.next_conduit_;
        } else
            return static_cast<reconduits::Conduit*>( nullptr );
    }

protected:

    constexpr auto accept_(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        using namespace reconduits;
        auto& emsg = msg.get();
        if( ( ctx_ptr_ = select_context( emsg ) ) ) {

            auto prev_state = update_connection_state( emsg );

            if( is_connection_established( prev_state ) ) {
                return std::pair{ NextSide::b, make_variant_message( msg ) };
            } else if( is_connection_closed() ) {
                return std::pair{ NextSide::b0, make_variant_release_message(msg, conduit_origin) };
            } else {
                if( is_connection_establishing( prev_state ) ) emsg.set_connection_established();
                return std::pair{ NextSide::b0, make_variant_setup_message(msg, conduit_origin) };
            }
        } else {
            create_context( emsg );
            return std::pair{ NextSide::b, make_variant_message( msg ) };
        }
    }

    struct key_hash : public std::unary_function<key_type, std::size_t>
    {
       std::size_t operator()(const key_type& k) const
       {
          return std::get<0>( k ) ^ std::get<1>( k ) ^
                 std::get<2>( k ) ^ std::get<3>( k );
       }
    };

    struct key_equal : public std::binary_function<key_type, key_type, bool>
    {
       bool operator()(const key_type& v0, const key_type& v1) const
       {
          return  std::get<0>( v0 ) == std::get<0>( v1 ) &&
                  std::get<1>( v0 ) == std::get<1>( v1 ) &&
                  std::get<2>( v0 ) == std::get<2>( v1 ) &&
                  std::get<3>( v0 ) == std::get<3>( v1 );
       }
    };

    struct ConnectionContext
    {
        reconduits::Conduit* next_conduit_;
        mock_state_machine::TCPStateMachine connection_state_;
    };

    auto select_context(const auto& emsg)
    {
        auto it = mux_table_.find( std::get<mock_packet::Packet::l4_id_type>( emsg.getL4Id() ) );
        return it != mux_table_.cend() ? &it->second : nullptr;
    }

    auto create_context(const auto& emsg)
    {
        auto [it, inserted] = mux_table_.emplace(std::get<mock_packet::Packet::l4_id_type>( emsg.getL4Id() ), ConnectionContext{nullptr, mock_state_machine::TCPStateMachine{}});
        if( inserted ) {
            const auto& pkt = emsg.packet();
            it->second.connection_state_.transition( mock_state_machine::Event{ pkt.is_ack(), pkt.is_syn(), pkt.is_fin(), pkt.is_timeout() } );
        }
    }

    constexpr bool is_connection_establishing(mock_state_machine::TCPStateMachine prev_state) const noexcept
    {
        return ctx_ptr_->connection_state_.is_current<mock_state_machine::Established>() &&
               prev_state.is_current<mock_state_machine::SynReceived>();
    }

    constexpr bool is_connection_established(mock_state_machine::TCPStateMachine prev_state) const noexcept
    {
        return ctx_ptr_->connection_state_.is_current<mock_state_machine::Established>() &&
               prev_state.is_current<mock_state_machine::Established>();
    }

    constexpr bool is_connection_closed() const noexcept
    {
        return ctx_ptr_->connection_state_.is_current<mock_state_machine::Closed>();
    }

    auto update_connection_state(auto&& emsg)
    {
        const auto& pkt = emsg.packet();
        return ctx_ptr_->connection_state_.transition( mock_state_machine::Event{ pkt.is_ack(), pkt.is_syn(), pkt.is_fin(), pkt.is_timeout() } );
    }

    using mux_table_type = std::unordered_map<key_type, ConnectionContext, key_hash, key_equal>;
    mux_table_type mux_table_;
    ConnectionContext* ctx_ptr_ = nullptr;
};

class L4LUAMux : public L4Mux
{
public:

    constexpr auto accept(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        SPDLOG_DEBUG(getLogger(), "L4LUAMux [{:p}] accepts a new message.", static_cast<void*>(this));
        auto& emsg = msg.get();
        lua.set_function("append", [ & ]{ emsg.append( "L4LUAMux" ); });
        lua.script("append()");

        return accept_(msg, conduit_origin);
    }

private:

    sol::state lua;
};

}
