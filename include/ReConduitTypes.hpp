#ifndef __CONDUIT_TYPES__HPP__
#define __CONDUIT_TYPES__HPP__

#include "ReConduitLogger.hpp"
#include "ReConduitTypesGenerators.hpp"

namespace reconduits {

class Protocol;
class Adapter;
class Mux;
class Factory;

using conduit_type = generate_type_from<Protocol, Adapter, Mux, Factory>;

}

#include "MessageTypes.hpp"
#include "ReConduitDispacher.hpp"
#include "ProtocolReConduit.hpp"
#include "MuxReConduit.hpp"
#include "AdapterReConduit.hpp"
#include "FactoryReConduit.hpp"


#endif //__CONDUIT_TYPES__HPP__
