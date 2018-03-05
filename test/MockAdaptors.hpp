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
        return emsg.isUpLink() ? reconduits::NextSide::a : reconduits::NextSide::done;
    }
};

struct ApplicationAdapter
{
    constexpr auto accept(auto&& msg)
    {
        auto& emsg = msg.get();
        emsg.append( "ApplicationAdapter" );
        return ! emsg.isUpLink() ? reconduits::NextSide::a : reconduits::NextSide::done;
    }
};

}
