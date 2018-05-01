#pragma once

#include "MockAdaptors.hpp"
#include "MockMuxes.hpp"
#include "MockProtocols.hpp"
#include "MockFactories.hpp"
#include "ReConduitTypesGenerators.hpp"

//////////////////////////////////////
// Conduit Types
//////////////////////////////////////

GENERATE_ADAPTER_CONDUITS(  mock_conduits::NetworkAdapter, mock_conduits::EndPointAdapter );
GENERATE_FACTORY_CONDUITS(  mock_conduits::NetworkFactory,  mock_conduits::ConnectionFactory );
GENERATE_MUX_CONDUITS(      mock_conduits::L3Mux, mock_conduits::L4Mux );
GENERATE_PROTOCOL_CONDUITS( mock_conduits::NetworkProtocol, mock_conduits::TCPProtocol, mock_conduits::UDPProtocol, mock_conduits::HTTPProtocol );

#include "ReConduitTypes.hpp"

