#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "MockPacket.hpp"
#include "MockMessage.hpp"

#include "MockConduitTypes.hpp"
#include "sol/sol.hpp"

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

    // HTTP Connection

    using namespace mock_packet;
    const Packet tcp_http_packets[] = {
    // tcp_http_packets[1] > SYN
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 80, TCPHeader::set_syn_flag() }
        },
    // tcp_http_packets[2] < SYN_ACK
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 80, 55000, TCPHeader::set_syn_ack_flags() }
        },
    // tcp_http_packets[3] > ACK
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 80, TCPHeader::set_ack_flag() }
        },
    // tcp_http_packets[4] > ACK [ GET URL ]
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 80, TCPHeader::set_ack_flag() },
          HTTPHeader{ "http://www.recoduit.cxm/" }
        },
    // tcp_http_packets[5] < ACK [ 200 OK ]
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 80, 55000, TCPHeader::set_ack_flag() },
          HTTPHeader{ "200 OK" }
        },
    // tcp_http_packets[6] > FIN ACK
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 80, TCPHeader::set_fin_flag() }
        },
    // tcp_http_packets[7] < ACK
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 80, 55000, TCPHeader::set_ack_flag() }
        },
    // tcp_http_packets[8] < FIN
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 80, 55000, TCPHeader::set_fin_flag() }
        },
    // tcp_http_packets[9] > ACK
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 80, TCPHeader::set_ack_timeout_flags() }
        }
    };

    // TLS Connection

    using namespace mock_packet;
    const Packet tcp_tls_packets[] = {
    // tcp_tls_packets[1] > SYN
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 443, TCPHeader::set_syn_flag() }
        },
    // tcp_tls_packets[2] < SYN_ACK
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 443, 55000, TCPHeader::set_syn_ack_flags() }
        },
    // tcp_tls_packets[3] > ACK
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 443, TCPHeader::set_ack_flag() }
        },
    // tcp_tls_packets[4] > ACK [ CLIENT HELLO ]
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 443, TCPHeader::set_ack_flag() },
          TLSHeader{ "www.recoduit.cxm" }
        },
    // tcp_tls_packets[5] < ACK [ SERVER HELLO ]
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 443, 55000, TCPHeader::set_ack_flag() },
          TLSHeader{ "http2" }
        },
    // tcp_tls_packets[6] > FIN ACK
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 443, TCPHeader::set_fin_flag() }
        },
    // tcp_tls_packets[7] < ACK
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 443, 55000, TCPHeader::set_ack_flag() }
        },
    // tcp_tls_packets[8] < FIN
        {
          IPv4Header{ "200.100.90.80", "10.11.12.13", ProtocolType::tcp }, // Down
          TCPHeader{ 443, 55000, TCPHeader::set_fin_flag() }
        },
    // tcp_tls_packets[9] > ACK
        {
          IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp }, // Up
          TCPHeader{ 55000, 443, TCPHeader::set_ack_timeout_flags() }
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

    for(auto i = 0u; i < sizeof tcp_http_packets / sizeof tcp_http_packets[0]; ++i) {

        // HTTP
        EXPECT_EQ( tcp_http_packets[i].get_src_port(), (uplinks[i] ? 55000 : 80));
        EXPECT_EQ( tcp_http_packets[i].get_dst_port(), (uplinks[i] ? 80 : 55000));
        if( i == 3 ) { EXPECT_EQ(tcp_http_packets[i].get_app_proto<HTTPHeader>().get_url(), "http://www.recoduit.cxm/"); }
        if( i == 4 ) { EXPECT_EQ(tcp_http_packets[i].get_app_proto<HTTPHeader>().get_response_code(), "200 OK"); }

        auto now = chrono::system_clock::now();
        Message tcp_http_msg{now, tcp_http_packets[i], uplinks[i]};
        network_adapter.accept( InformationChunk<Message>{ tcp_http_msg } );
        std::cout << i + 1 << ".- HTTP packet:\n" << tcp_http_msg;

        // TLS
        EXPECT_EQ( tcp_tls_packets[i].get_src_port(), (uplinks[i] ? 55000 : 443));
        EXPECT_EQ( tcp_tls_packets[i].get_dst_port(), (uplinks[i] ? 443 : 55000));
        if( i == 3 ) { EXPECT_EQ(tcp_tls_packets[i].get_app_proto<TLSHeader>().get_server_name_idication(), "www.recoduit.cxm"); }
        if( i == 4 ) { EXPECT_EQ(tcp_tls_packets[i].get_app_proto<TLSHeader>().get_application_layer_protocol_negociation(), "http2"); }

        now = chrono::system_clock::now();
        Message tcp_tls_msg{now, tcp_tls_packets[i], uplinks[i]};
        network_adapter.accept( InformationChunk<Message>{ tcp_tls_msg } );
        std::cout << i + 1 << ".- TLS packet:\n" << tcp_tls_msg;
    }

    // UDP connection example

//    Packet udp_packet {
//        IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::udp },
//        UDPHeader{ 55000, 80 },
//        DNSHeader{ "www.recoduit.cxm" }};
//
//    EXPECT_EQ( udp_packet.get_src_port(), 55000);
//    EXPECT_EQ( udp_packet.get_dst_port(), 80);
//    EXPECT_EQ(udp_packet.get_app_proto<DNSHeader>().get_uri(), "www.recoduit.cxm");
//
//    auto now = chrono::system_clock::now();
//    Message udp_msg{now, udp_packet, true};
//    network_adapter.accept( InformationChunk<Message>{ udp_msg } );
//
//    //network_adapter.accept( Release<Message>{udp_msg, &network_adapter} );
//    std::cout << udp_msg;
}

