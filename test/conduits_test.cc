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

    Conduit a1{ Adapter{ NetworkAdapter{} } }, a2{ Adapter{ EndPointAdapter{} } };
    Conduit p1{ Protocol{ IPProtocol{} } };
    Conduit m1{ Mux{ ConnectionsMux{} } };
    //Conduit m1{ Mux{ ConnectionsLUAMux{} } };
    Conduit f{ Factory{ ConnectionFactory{} } };

    a1.setSideA( p1 );

    p1.setSideB( m1 );

    m1.setSideB( f );

    f.setSideA( m1 );
    f.setSideB( a2 );

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
    a1.accept( InformationChunk<Message>{ msg } );
    a1.accept( Release<Message>{msg, &a1} );

    std::cout << msg;
}

#if 0
TEST(ConduitTest, MockDPIexample) {

    using namespace std;
    using namespace reconduits;
    using namespace mock_conduits;


    Conduit a1{ Adapter{ NetworkAdapter{} } }, a2{ Adapter{ ApplicationAdapter{} } };
    Conduit p1{ Protocol{ IPProtocol{} } };
    Conduit m1{ Mux{ ConnectionsLUAMux{} } }, m2{ Mux{ ConnectionsMux{} } };
    Conduit f{ Factory{ ConnectionFactory{} } };

    a1.setSideA( p1 );

    p1.setSideA( a1 );
    p1.setSideB( m1 );

    m1.setSideA( p1 );
    m1.setSideB( f );

    f.setSideA( m1 );
    f.setSideB( m2 );

    m2.setSideA( a2 );
    m2.setSideB( f );

    a2.setSideA( m2 );

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
    a1.accept( InformationChunk<Message>{ msg } );
}

TEST(ConduitTest, MockDPI) {

    using namespace std;
    using namespace reconduits;
    using namespace mock_conduits;

    Conduit a1{ Adapter{ NetworkAdapter{} } }, a2{ Adapter{ ApplicationAdapter{} } };
    Conduit p1{ Protocol{ IPProtocol{} } };
    Conduit m1{ Mux{ ConnectionsLUAMux{} } }, m2{ Mux{ ConnectionsMux{} } };
    Conduit f{ Factory{ TCPConnectionFactory{} } };

    a1.setSideA( p1 );

    p1.setSideA( a1 );
    p1.setSideB( m1 );

    m1.setSideA( p1 );
    m1.setSideB( f );

    f.setSideA( m1 );
    f.setSideB( m2 );

    m2.setSideA( a2 );
    m2.setSideB( f );

    a2.setSideA( m2 );

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
    a1.accept( InformationChunk<Message>{ msg } );
}
#endif
