#include "MockConduitTypes.hpp"

namespace mock_conduits {

//////////////////////////////////////
// Factories definitions
//////////////////////////////////////

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

    auto key = emsg.getId();
    return a->insertInSideB(key, p2) ? &p2 : nullptr;
}

reconduits::Conduit* ConnectionFactory::clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    auto& emsg = msg.get();
    emsg.append( "ConnectionFactory: Release" );
    auto key = emsg.getId();
    using namespace reconduits;
    putToPool<sizeof(Conduit)>( a->eraseFromSideB( key ) );
    putToPool<sizeof(Conduit)>( b->eraseFromSideB( key ) );
    return b;
}

}
