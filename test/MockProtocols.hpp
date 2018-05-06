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
// Protocols
//////////////////////////////////////

struct NetworkProtocol
{
    constexpr auto accept(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        using namespace reconduits;
        auto& emsg = msg.get();
        emsg.append( "NetworkProtocol" );
        return std::pair{ NextSide::b, make_variant_message( msg ) };
    }
};

struct TCPProtocol
{
    constexpr auto accept(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        using namespace reconduits;
        auto& emsg = msg.get();
        emsg.append( "TCPProtocol" );

        update_connection_state( emsg );

        if( state_.is_current<mock_state_machine::Established>() ) {
            emsg.set_connection_established();
            return std::pair{ NextSide::b, make_variant_setup_message(msg, conduit_origin) };
        } else if( state_.is_current<mock_state_machine::Closed>() )
            return std::pair{ NextSide::b, make_variant_release_message(msg, conduit_origin) };
        else
            return std::pair{ NextSide::b, make_variant_message( msg ) };
    }

private:

    void update_connection_state(auto&& emsg)
    {
        const auto& pkt = emsg.packet();
        state_.transition( mock_state_machine::Event{ pkt.is_ack(), pkt.is_syn(), pkt.is_fin(), pkt.is_timeout() } );
    }

    mock_state_machine::TCPStateMachine state_;
};

struct UDPProtocol
{
    constexpr auto accept(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        using namespace reconduits;
        auto& emsg = msg.get();
        emsg.append( "UDPProtocol" );
        return std::pair{ NextSide::b, make_variant_message( msg ) };
    }
};

struct HTTPProtocol
{
    constexpr auto accept(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        using namespace reconduits;
        auto& emsg = msg.get();
        emsg.append( "HTTPProtocol" );
        return std::pair{ NextSide::b, make_variant_release_message(msg, conduit_origin) };
    }
};

struct TLSProtocol
{
    constexpr auto accept(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        using namespace reconduits;
        auto& emsg = msg.get();
        emsg.append( "TLSProtocol" );
        return std::pair{ NextSide::b, make_variant_release_message(msg, conduit_origin) };
    }
};

struct DNSProtocol
{
    constexpr auto accept(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        using namespace reconduits;
        auto& emsg = msg.get();
        emsg.append( "DNSProtocol" );
        return std::pair{ NextSide::b, make_variant_release_message(msg, conduit_origin) };
    }
};
}
