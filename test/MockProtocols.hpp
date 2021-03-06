#pragma once

#include "ReConduitTypesGenerators.hpp"
#include "MockLogger.hpp"
#include "MockMessage.hpp"

#include "sol/sol.hpp"

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
        return std::pair{ NextSide::b, make_variant_message( msg ) };
    }
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
