#pragma once

#include "ReConduitTypesGenerators.hpp"
#include "MockLogger.hpp"
#include "MockMessage.hpp"

#include "sol.hpp"

#include <unordered_map>
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

    constexpr auto accept(auto&& msg)
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

    using key_type   = typename Message::key_type;
    using value_type = reconduits::Conduit*;

    constexpr auto accept(auto&& msg)
    {
        using namespace reconduits;
        SPDLOG_DEBUG(getLogger(), "L4Mux [{:p}] accepts a new message.", static_cast<void*>(this));
        auto& emsg = msg.get();
        emsg.append( "L4Mux" );
        using T = std::decay_t<decltype(msg)>;
        if constexpr (std::is_same_v<T, reconduits::Release<Message>>)
            return std::pair{ NextSide::b0, make_variant_message( msg ) };
        else
            return std::pair{ NextSide::b,  make_variant_message( msg ) };
    }

    auto find(auto&& msg) const
    {
        auto it = mux_table_.find( std::get<mock_packet::Packet::l4_id_type>( msg.get().getL4Id() ) );
        bool found = it != mux_table_.cend();
        SPDLOG_DEBUG(getLogger(), "L4Mux [{0:p}] has{1}found key {2}.", static_cast<const void*>(this), (found ? " ":" NOT "),
                std::get<mock_packet::Packet::l4_id_type>( msg.get().getL4Id() ));
        return std::pair{(found ? it->second : nullptr), found};
    }

    auto insert(auto&& key, reconduits::Conduit& c)
    {
        SPDLOG_DEBUG(getLogger(), "L4Mux [{:p}] is going to connect new conduits.", static_cast<void*>(this));
        auto [it, inserted] = mux_table_.emplace(std::get<mock_packet::Packet::l4_id_type>( key ), &c);
        if( inserted ) return it->second;
        else {
            getLogger()->warn("The new connection for key {} was not possible.", std::get<mock_packet::Packet::l4_id_type>( key ));
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
            return it->second;
        } else
            return static_cast<reconduits::Conduit*>( nullptr );
    }

protected:

    using mux_table_type = std::unordered_map<key_type, reconduits::Conduit*>;
    mux_table_type mux_table_;
};

class L4LUAMux : public L4Mux
{
public:
    constexpr auto accept(auto&& msg)
    {
        using namespace reconduits;
        SPDLOG_DEBUG(getLogger(), "L4LUAMux [{:p}] accepts a new message.", static_cast<void*>(this));
        auto& emsg = msg.get();
        lua.set_function("append", [ & ]{ emsg.append( "L4LUAMux" ); });
        lua.script("append()");
        return std::pair{ NextSide::b, make_variant_message( msg ) };
    }

private:
    sol::state lua;
};

}
