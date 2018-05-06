#include "MockConduitTypes.hpp"

namespace mock_conduits {

//////////////////////////////////////
// Factories definitions
//////////////////////////////////////

reconduits::Conduit* NetworkFactory::create_tcp_connection(reconduits::Setup<Message>& msg, reconduits::Conduit*a, reconduits::Conduit* b) const
{
    //   ___________                              _____________
    //  /           |                            /        [b1] |
    // | l3_mux [bi]| --> | tcp_parser [b]| --> | l4_mux  [b2] |
    //  \___________|                            \_[b0]___[bn]_|
    //                                               ^
    //                                               |
    //                                               +-> |[a] connection_factoy [b]| --> | endpoint_adapter |

    using namespace reconduits;
    auto tcp_parser        = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ TCPProtocol{} } };
    auto l4_mux            = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Mux{ L4LUAMux{} } };
    auto connection_factoy = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Factory{ TCPConnectionFactory{} } };

    tcp_parser->setSideB( *l4_mux );
    l4_mux->setSideB( *connection_factoy );

    connection_factoy->setSideA( *l4_mux );
    connection_factoy->setSideB( *b );

    auto& emsg = msg.get();
    emsg.append( "NetworkFactory: Setup Conduits to cope with TCP connections" );

    auto key = emsg.getL3Id();
    a->insertInSideB(key, *tcp_parser);
    return tcp_parser;
}

reconduits::Conduit* NetworkFactory::create_udp_connection(reconduits::Setup<Message>& msg, reconduits::Conduit*a, reconduits::Conduit* b) const
{
    //   ___________                              _____________
    //  /           |                            /        [b1] |
    // | l3_mux [bi]| --> | udp_parser [b]| --> | l4_mux  [b2] |
    //  \___________|                            \_[b0]___[bn]_|
    //                                               ^
    //                                               |
    //                                               +-> |[a] connection_factoy [b]| --> | endpoint_adapter |

    using namespace reconduits;
    auto udp_parser        = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ UDPProtocol{} } };
    auto l4_mux            = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Mux{ L4Mux{} } };
    auto connection_factoy = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Factory{ UDPConnectionFactory{} } };

    udp_parser->setSideB( *l4_mux );
    l4_mux->setSideB( *connection_factoy );

    connection_factoy->setSideA( *l4_mux );
    connection_factoy->setSideB( *b );

    auto& emsg = msg.get();
    emsg.append( "NetworkFactory: Setup Conduits to cope with UDP connections" );

    auto key = emsg.getL3Id();
    a->insertInSideB(key, *udp_parser);
    return udp_parser;
}

bool TCPConnectionFactory::is_l4_connection_established(reconduits::Setup<Message>& msg) const
{
    return msg.get().connection_established();
}

reconduits::Conduit* TCPConnectionFactory::select_application_protocol(reconduits::Setup<Message>& msg) const
{
    switch( msg.get().app_proto() ) {
        case Message::http_app_protocol:
        {
            //   ___________
            //  /           |
            // | l4_mux [bi]| --> | http_parser [b]| --> | endpoint_adapter |
            //  \__[b0]_____|                                    ^
            //      ^                                            |
            //      |                                            |
            //      +-> |[a] connection_factoy [b]| -------------+

            using namespace reconduits;
            auto& emsg = msg.get();
            emsg.append( "TCPConnectionFactory: Setup HTTP connection" );
            return new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ HTTPProtocol{} } };
        }
        case Message::tls_app_protocol:
        {
            //   ___________
            //  /           |
            // | l4_mux [bi]| --> | tls_parser [b]| --> | endpoint_adapter |
            //  \__[b0]_____|                                    ^
            //      ^                                            |
            //      |                                            |
            //      +-> |[a] connection_factoy [b]| -------------+

            using namespace reconduits;
            auto& emsg = msg.get();
            emsg.append( "TCPConnectionFactory: Setup TLS connection" );
            return new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ TLSProtocol{} } };
        }
        default: return nullptr;
    }
}

reconduits::Conduit* TCPConnectionFactory::create(reconduits::Setup<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    if( is_l4_connection_established( msg ) ) {
        if( auto application_protocol_parser = select_application_protocol(msg) ) {
            application_protocol_parser->setSideB( *b );
            a->insertInSideB(msg.get().getL4Id(), *application_protocol_parser);
            return application_protocol_parser;
        }
    }

    //   ___________
    //  /           |
    // | l4_mux [bi]| -------------------------> | endpoint_adapter |
    //  \__[b0]_____|                                    ^
    //      ^                                            |
    //      |                                            |
    //      +-> |[a] connection_factoy [b]| -------------+

    return b;
}

reconduits::Conduit* TCPConnectionFactory::clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    auto& emsg = msg.get();
    switch( msg.get().app_proto() ) {
        case Message::http_app_protocol:
        {
            emsg.append( "TCPConnectionFactory: Release HTTP connection" );
            break;
        }
        case Message::tls_app_protocol:
        {
            emsg.append( "TCPConnectionFactory: Release TLS connection" );
            break;
        }
        default:
        {
            emsg.append( "TCPConnectionFactory: Release Unkown connection" );
            break;
        }
    }
    auto key = emsg.getL4Id();
    using namespace reconduits;
    putToPool<sizeof(Conduit)>( a->eraseFromSideB( key ) );
    return b;
}

reconduits::Conduit* UDPConnectionFactory::create(reconduits::Setup<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    //   ___________
    //  /           |
    // | l4_mux [bi]| --> | dns_parser [b]| --> | endpoint_adapter |
    //  \__[b0]_____|                                    ^
    //      ^                                            |
    //      |                                            |
    //      +-> |[a] connection_factoy [b]| -------------+

    using namespace reconduits;
    auto dns_parser = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ DNSProtocol{} } };

    dns_parser->setSideB( *b );

    auto& emsg = msg.get();
    emsg.append( "UDPConnectionFactory: Setup DNS connection" );

    auto key = emsg.getL4Id();
    a->insertInSideB(key, *dns_parser);
    return dns_parser;
}

reconduits::Conduit* UDPConnectionFactory::clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    auto& emsg = msg.get();
    emsg.append( "UDPConnectionFactory: Release DNS connection" );
    auto key = emsg.getL4Id();
    using namespace reconduits;
    putToPool<sizeof(Conduit)>( a->eraseFromSideB( key ) );
    return b;
}

}
