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
// Adaptors
//////////////////////////////////////

struct NetworkAdapter
{
    constexpr auto accept(auto&& msg)
    {
        auto& emsg = msg.get();
        emsg.append( "NetworkAdapter" );
        return std::pair{ reconduits::NextSide::a, make_variant_message( msg ) };
    }
};

struct EndPointAdapter
{
    constexpr auto accept(auto&& msg)
    {
        auto& emsg = msg.get();
        emsg.append( "EndPointAdapter" );
        return std::pair{ reconduits::NextSide::done, make_variant_message( msg ) };
    }
};

}
