#include "MockConduitTypes.hpp"

namespace mock_conduits {

//////////////////////////////////////
// Factories definitions
//////////////////////////////////////

reconduits::Conduit* NetworkFactory::create_tcp_connection(reconduits::Setup<Message>& msg, reconduits::Conduit*a, reconduits::Conduit* b) const
{
    using namespace reconduits;
    auto tcp_parser        = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ TCPProtocol{} } };
    auto l4_mux            = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Mux{ L4Mux{} } };
    auto connection_factoy = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Factory{ TCPConnectionFactory{} } };

    tcp_parser->setSideB( *l4_mux );
    l4_mux->setSideB( *connection_factoy );

    connection_factoy->setSideA( *l4_mux );
    connection_factoy->setSideB( *b );

    auto& emsg = msg.get();
    emsg.append( "NetworkFactory: Setup TCP connection" );

    auto key = emsg.getL3Id();
    a->insertInSideB(key, *tcp_parser);
    return tcp_parser;
}

reconduits::Conduit* NetworkFactory::create_udp_connection(reconduits::Setup<Message>& msg, reconduits::Conduit*a, reconduits::Conduit* b) const
{
    using namespace reconduits;
    auto udp_parser        = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ UDPProtocol{} } };
    auto l4_mux            = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Mux{ L4Mux{} } };
    auto connection_factoy = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Factory{ UDPConnectionFactory{} } };

    udp_parser->setSideB( *l4_mux );
    l4_mux->setSideB( *connection_factoy );

    connection_factoy->setSideA( *l4_mux );
    connection_factoy->setSideB( *b );

    auto& emsg = msg.get();
    emsg.append( "NetworkFactory: Setup UDP connection" );

    auto key = emsg.getL3Id();
    a->insertInSideB(key, *udp_parser);
    return udp_parser;
}

reconduits::Conduit* TCPConnectionFactory::create(reconduits::Setup<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    using namespace reconduits;
    auto http_parser = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ HTTPProtocol{} } };

    http_parser->setSideB( *b );

    auto& emsg = msg.get();
    emsg.append( "ConnectionFactory: Setup HTTP connection" );

    auto key = emsg.getL4Id();
    a->insertInSideB(key, *http_parser);
    return http_parser;
}

reconduits::Conduit* TCPConnectionFactory::clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    auto& emsg = msg.get();
    emsg.append( "ConnectionFactory: Release HTTP connection" );
    auto key = emsg.getL4Id();
    using namespace reconduits;
    putToPool<sizeof(Conduit)>( a->eraseFromSideB( key ) );
    return b;
}

reconduits::Conduit* UDPConnectionFactory::create(reconduits::Setup<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    using namespace reconduits;
    auto dns_parser = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ DNSProtocol{} } };

    dns_parser->setSideB( *b );

    auto& emsg = msg.get();
    emsg.append( "ConnectionFactory: Setup DNS connection" );

    auto key = emsg.getL4Id();
    a->insertInSideB(key, *dns_parser);
    return dns_parser;
}

reconduits::Conduit* UDPConnectionFactory::clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    auto& emsg = msg.get();
    emsg.append( "ConnectionFactory: Release DNS connection" );
    auto key = emsg.getL4Id();
    using namespace reconduits;
    putToPool<sizeof(Conduit)>( a->eraseFromSideB( key ) );
    return b;
}

}
