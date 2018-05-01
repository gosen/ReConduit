#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "MockPacket.hpp"
#include "MockMessage.hpp"

#include "MockConduitTypes.hpp"
#include "sol.hpp"

#include <unordered_map>
#include <type_traits>
#include <string>
#include <sstream>

inline auto getLogger(std::string logger_name = "reconduit_test")
{
    static auto logger{ spdlog::stdout_color_mt( logger_name ) };
    spdlog::set_level(spdlog::level::warn);
    return logger;
}

template<typename T> struct PrintType;

//////////////////////////////////////
// Tests
//////////////////////////////////////

TEST(ConduitTest, MockDPIexample) {

    using namespace std;
    using namespace reconduits;
    using namespace mock_conduits;

    /////////////////////////////////////////////////////////////////
    // Conduits to be used
    /////////////////////////////////////////////////////////////////

    // Adapters:
    Conduit network_adapter{ Adapter{ NetworkAdapter{} } };
    Conduit endpoint_adapter{ Adapter{ EndPointAdapter{} } };

    // Protocols:
    Conduit network_protocol{ Protocol{ NetworkProtocol{} } };
//    Conduit transport_protocol{ Protocol{ TransportProtocol{} } };
//    Conduit udp_protocol{ Protocol{ UDPProtocol{} } };
//    Conduit tcp_protocol{ Protocol{ TCPProtocol{} } };
//    Conduit dns_protocol{ Protocol{ DNSProtocol{} } };
//    Conduit http_protocol{ Protocol{ HTTPProtocol{} } };

    // Muxes:
    Conduit l3_mux{ Mux{ L3Mux{} } };
//    Conduit l4_mux{ Mux{ L4Mux{} } };
//    Conduit l7_udp_mux{ Mux{ L7Mux{} } };
//    Conduit l7_tcp_mux{ Mux{ L7Mux{} } };

    // Factories:
    Conduit network_factory{ Factory{ NetworkFactory{} } };
//    Conduit conections_factory{ Factory{ ConnectionFactory{} } };
//    Conduit application_factory{ Factory{ ApplicationFactory{} } };

    /////////////////////////////////////////////////////////////////
    // Conduits interconnections
    /////////////////////////////////////////////////////////////////


    //                                                            _____________
    //                                                           /        [b1] |
    // | network_adapter [a] | --> | network_protocol [b] | --> | l3_mux  [b2] |
    //                                                           \_[b0]___[bn]_|
    //                                                              ^
    //                                                              |
    //                                                              +-----> |[a] network_factory [b] | ---> | endpoint_adapter |

    network_adapter.setSideA( network_protocol );
    network_protocol.setSideB( l3_mux );
    l3_mux.setSideB( network_factory );
    network_factory.setSideA( l3_mux );
    network_factory.setSideB( endpoint_adapter );

    using namespace mock_packet;
    Packet packet {
        IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp },
        TCPHeader{ 55000, 80, TCPHeader::set_ack_flag() },
        HTTPHeader{ "http://www.recoduit.cxm/" }};

    EXPECT_EQ(ntohs( packet.get_src_port() ), 55000);
    EXPECT_EQ(ntohs( packet.get_dst_port() ), 80);
    EXPECT_EQ(packet.get_app_proto<HTTPHeader>().get_url(), "http://www.recoduit.cxm/");

    auto now = chrono::system_clock::now();
    Message msg{now, packet, true};
    network_adapter.accept( InformationChunk<Message>{ msg } );
    network_adapter.accept( Release<Message>{msg, &network_adapter} );

    std::cout << msg;
}

