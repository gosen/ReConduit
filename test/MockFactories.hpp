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
// Factories
//////////////////////////////////////

class NetworkFactory
{
public:

    constexpr reconduits::Conduit* accept(auto&& msg, reconduits::Conduit* a, reconduits::Conduit* b)
    {
        using T = std::decay_t<decltype(msg)>;
        if constexpr (std::is_same_v<T, reconduits::Setup<Message>>) {
            switch( std::get<mock_packet::ProtocolType>( msg.get().getL3Id() ) ) {
                case mock_packet::ProtocolType::tcp: return create_tcp_connection(msg, a, b);
                case mock_packet::ProtocolType::udp: return create_udp_connection(msg, a, b);
                default: return b;
            }
        } else if constexpr (std::is_same_v<T, reconduits::Release<Message>>) {
            return nullptr; // Do nothing
        } else {
            getLogger()->warn("Useless message type has reached this[{:p}] ConnectionFactory", static_cast<const void*>(this));
            return nullptr;
        }
    }

private:

    reconduits::Conduit* create_tcp_connection(reconduits::Setup<Message>& msg, reconduits::Conduit*a, reconduits::Conduit* b) const;
    reconduits::Conduit* create_udp_connection(reconduits::Setup<Message>& msg, reconduits::Conduit*a, reconduits::Conduit* b) const;
};

template<typename D>
struct ConnectionFactory
{
    constexpr reconduits::Conduit* accept(auto&& msg, reconduits::Conduit* a, reconduits::Conduit* b)
    {
        using T = std::decay_t<decltype(msg)>;
        if      constexpr (std::is_same_v<T, reconduits::Setup<Message>>)  return static_cast<D*>( this )->create(msg, a, b);
        else if constexpr (std::is_same_v<T, reconduits::Release<Message>>) return static_cast<D*>( this )->clean(msg, a, b);
        else {
            getLogger()->warn("Useless message type has reached this[{:p}] ConnectionFactory", static_cast<const void*>(this));
            return nullptr;
        }
    }
};

class TCPConnectionFactory : public ConnectionFactory<TCPConnectionFactory>
{
private:

    friend ConnectionFactory<TCPConnectionFactory>;

    reconduits::Conduit* create(reconduits::Setup<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b);
    reconduits::Conduit* clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b);

    bool is_l4_connection_established(reconduits::Setup<Message>& msg) const;
    reconduits::Conduit* select_application_protocol(reconduits::Setup<Message>& msg) const;
};

class UDPConnectionFactory : public ConnectionFactory<UDPConnectionFactory>
{
private:

    friend ConnectionFactory<UDPConnectionFactory>;

    reconduits::Conduit* create(reconduits::Setup<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b);
    reconduits::Conduit* clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b);
};

}
