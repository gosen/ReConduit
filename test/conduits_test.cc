#include "gtest/gtest.h"
#include "spdlog/spdlog.h"
#include "MockPacket.hpp"

#include "MockConduitTypes.hpp"
#include "sol.hpp"

#include <unordered_map>
#include <type_traits>
#include <string>
#include <sstream>

inline auto getLogger(std::string logger_name = "reconduit_test")
{
    static auto logger{ spdlog::stdout_color_mt( logger_name ) };
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

    spdlog::set_level(spdlog::level::warn);
    auto logger = spdlog::stdout_color_mt("conduits_test");

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

#if 0
        stringstream ss_req, ss_conduit_req;
        Message greeting{msg_cnt, "Hello word!!!", true};
        logger->info( " -------------- {} ------------------ ", msg_cnt );
        a1.accept( InformationChunk<Message>{ greeting } );
        greeting( ss_conduit_req );
        ss_req << "Message Id: " << msg_cnt << ". Uplink message: Hello word!!!\n" << request_msg;
        EXPECT_EQ(ss_req.str(), ss_conduit_req.str());
#endif

    using namespace mock_packet;
    Packet packet {
        IPv4Header{ "10.11.12.13", "200.100.90.80", ProtocolType::tcp },
        TCPHeader{ 55000, 80 },
        HTTPHeader{}};

    auto src_port = packet.get_src_port();
    EXPECT_EQ(ntohs( src_port ), 55000);
}
