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
GENERATE_FACTORY_CONDUITS(  mock_conduits::NetworkFactory, \
                             mock_conduits::TCPConnectionFactory, mock_conduits::UDPConnectionFactory );
GENERATE_MUX_CONDUITS(      mock_conduits::L3Mux, mock_conduits::L4Mux, mock_conduits::L4LUAMux );
GENERATE_PROTOCOL_CONDUITS( mock_conduits::NetworkProtocol, \
                            mock_conduits::TCPProtocol,  mock_conduits::UDPProtocol, \
                            mock_conduits::HTTPProtocol, mock_conduits::DNSProtocol );

#include "ReConduitTypes.hpp"

