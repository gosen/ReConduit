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

struct IPProtocol
{
    constexpr auto accept(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        using namespace reconduits;
        auto& emsg = msg.get();
        emsg.append( "IPProtocol" );
        return std::pair{ (emsg.isUpLink() ? NextSide::b : NextSide::a), make_variant_message( msg ) };
    }
};

struct TCPProtocol
{
    constexpr auto accept(auto&& msg, reconduits::Conduit* conduit_origin)
    {
        using namespace reconduits;
        auto& emsg = msg.get();
        emsg.append( "TCPProtocol" );
        return std::pair{ (emsg.isUpLink() ? NextSide::b : NextSide::a), make_variant_message( msg ) };
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

}
