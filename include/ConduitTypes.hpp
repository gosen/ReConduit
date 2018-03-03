#ifndef __CONDUIT_TYPES__HPP__
#define __CONDUIT_TYPES__HPP__

#include "ConduitLogger.hpp"
#include "ConduitTypesGenerators.hpp"

namespace conduits {

class Protocol;
class Adapter;
class Mux;
class Factory;

using conduit_type = generate_type_from<Protocol, Adapter, Mux, Factory>;

}

#include "MessageTypes.hpp"
#include "ConduitDispacher.hpp"
#include "ProtocolConduit.hpp"
#include "MuxConduit.hpp"
#include "AdapterConduit.hpp"
#include "FactoryConduit.hpp"


#endif //__CONDUIT_TYPES__HPP__
