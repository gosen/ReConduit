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
    constexpr auto accept(auto&& msg)
    {
        auto& emsg = msg.get();
        emsg.append( "IPProtocol" );
        return std::pair{ (emsg.isUpLink() ? reconduits::NextSide::b : reconduits::NextSide::a), &msg };
    }
};

struct TCPProtocol
{
    constexpr auto accept(auto&& msg)
    {
        auto& emsg = msg.get();
        emsg.append( "TCPProtocol" );
        return std::pair{ (emsg.isUpLink() ? reconduits::NextSide::b : reconduits::NextSide::a), &msg };
    }
};

struct HTTPProtocol
{
    constexpr auto accept(auto&& msg)
    {
        using namespace reconduits;
        using msg_type = message_type<embedded_t<decltype(msg)>>;
        auto& emsg = msg.get();
        if( emsg.isGoodBye() ) {
            emsg.append( "HTTPProtocol is closing this connection" );
            using release_msg_type = Release<embedded_t<decltype(msg)>>;
            auto& relese_msg = *new release_msg_type{emsg, nullptr};
            return std::pair{ (emsg.isUpLink() ? NextSide::b : NextSide::a), msg_type{ &relese_msg } };
        } else {
            emsg.append( "HTTPProtocol" );
            return std::pair{ (emsg.isUpLink() ? NextSide::b : NextSide::a), msg_type{ &msg } };
        }
    }
};

}
