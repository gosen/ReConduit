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

    // Muxes:
    Conduit l3_mux{ Mux{ L3Mux{} } };

    // Factories:
    Conduit network_factory{ Factory{ NetworkFactory{} } };

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

    // TCP connection example

    using namespace mock_packet;
    const Packet tcp_packets[] = {
    // tcp_packets[1] > SYN
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 80, TCPHeader::set_syn_flag() }
        },
    // tcp_packets[2] < SYN_ACK
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 80, 55000, TCPHeader::set_syn_ack_flags() }
        },
    // tcp_packets[3] > ACK
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 80, TCPHeader::set_ack_flag() }
        },
    // tcp_packets[4] > ACK [ GET URL ]
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 80, TCPHeader::set_ack_flag() },
          HTTPHeader{ "http://www.recoduit.cxm/" }
        },
    // tcp_packets[5] < ACK [ 200 OK ]
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 80, 55000, TCPHeader::set_ack_flag() },
          HTTPHeader{ "200 OK" }
        },
    // tcp_packets[6] > FIN ACK
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 80, TCPHeader::set_fin_flag() }
        },
    // tcp_packets[7] < ACK
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 80, 55000, TCPHeader::set_ack_flag() }
        },
    // tcp_packets[8] < FIN
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 80, 55000, TCPHeader::set_fin_flag() }
        },
    // tcp_packets[9] > ACK
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 80, TCPHeader::set_ack_timeout_flags() }
        }
    };

    bool uplinks[] = {
        true,  // 1 syn
        false, // 2 syn_ack
        true,  // 3 ack
        true,  // 4 GET
        false, // 5 200 OK
        true,  // 6 fin_ack
        false, // 7 ack
        false, // 8 fin_ack
        true,  // 9 fin
    };

    for(auto i = 0u; i < sizeof tcp_packets / sizeof tcp_packets[0]; ++i) {

        EXPECT_EQ(ntohs( tcp_packets[i].get_src_port() ), (uplinks[i] ? 55000 : 80));
        EXPECT_EQ(ntohs( tcp_packets[i].get_dst_port() ), (uplinks[i] ? 80 : 55000));
        if( i == 3 ) { EXPECT_EQ(tcp_packets[i].get_app_proto<HTTPHeader>().get_url(), "http://www.recoduit.cxm/"); }
        if( i == 4 ) { EXPECT_EQ(tcp_packets[i].get_app_proto<HTTPHeader>().get_url(), "200 OK"); }

        auto now = chrono::system_clock::now();
        Message tcp_msg{now, tcp_packets[i], uplinks[i]};
        network_adapter.accept( InformationChunk<Message>{ tcp_msg } );
        std::cout << tcp_msg;
    }

    // UDP connection example

    Packet udp_packet {
        IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::udp },
        UDPHeader{ 55000, 80 },
        DNSHeader{ "www.recoduit.cxm" }};

    EXPECT_EQ(ntohs( udp_packet.get_src_port() ), 55000);
    EXPECT_EQ(ntohs( udp_packet.get_dst_port() ), 80);
    EXPECT_EQ(udp_packet.get_app_proto<DNSHeader>().get_uri(), "www.recoduit.cxm");

    auto now = chrono::system_clock::now();
    Message udp_msg{now, udp_packet, true};
    network_adapter.accept( InformationChunk<Message>{ udp_msg } );

    //network_adapter.accept( Release<Message>{udp_msg, &network_adapter} );
    std::cout << udp_msg;
}

