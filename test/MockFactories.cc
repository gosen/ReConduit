#include "MockConduitTypes.hpp"

namespace mock_conduits {

//////////////////////////////////////
// Factories definitions
//////////////////////////////////////

reconduits::Conduit* NetworkFactory::tcp(reconduits::Conduit* b) const
{
    using namespace reconduits;
    auto tcp = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ TCPProtocol{} } };
    tcp->setSideB( *b );
    return tcp;
}

reconduits::Conduit* NetworkFactory::udp(reconduits::Conduit* b) const
{
    using namespace reconduits;
    auto udp = new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ UDPProtocol{} } };
    udp->setSideB( *b );
    return udp;
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
