#pragma once

#include "MockAdaptors.hpp"
#include "MockMuxes.hpp"
#include "MockProtocols.hpp"
#include "MockFactories.hpp"
#include "ReConduitTypesGenerators.hpp"

//////////////////////////////////////
// Conduit Types
//////////////////////////////////////

GENERATE_ADAPTER_CONDUITS(  mock_conduits::NetworkAdapter, mock_conduits::ApplicationAdapter );
GENERATE_FACTORY_CONDUITS(  mock_conduits::TCPConnectionFactory );
GENERATE_MUX_CONDUITS(      mock_conduits::ConnectionsMux, mock_conduits::ConnectionsLUAMux );
GENERATE_PROTOCOL_CONDUITS( mock_conduits::IPProtocol, mock_conduits::TCPProtocol, mock_conduits::HTTPProtocol );

#include "ReConduitTypes.hpp"

