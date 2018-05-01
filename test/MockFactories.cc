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
    auto connection_factoy = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Factory{ ConnectionFactory{} } };

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
    auto connection_factoy = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Factory{ ConnectionFactory{} } };

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

reconduits::Conduit* ConnectionFactory::create(reconduits::Setup<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    using namespace reconduits;
    auto& p2 = *new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ TCPProtocol{} } };
    auto& p3 = *new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ HTTPProtocol{} } };

    p2.setSideA( *a );
    p2.setSideB( p3 );

    p3.setSideA( p2 );
    p3.setSideB( *b );

    auto& emsg = msg.get();
    emsg.append( "ConnectionFactory: Setup" );

    auto key = emsg.getL4Id();
    return a->insertInSideB(key, p2) ? &p2 : nullptr;
}

reconduits::Conduit* ConnectionFactory::clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    auto& emsg = msg.get();
    emsg.append( "ConnectionFactory: Release" );
    auto key = emsg.getL4Id();
    using namespace reconduits;
    putToPool<sizeof(Conduit)>( a->eraseFromSideB( key ) );
    putToPool<sizeof(Conduit)>( b->eraseFromSideB( key ) );
    return b;
}

}
