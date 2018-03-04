#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

#include "ConduitTypesGenerators.hpp"
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
// Actual Message
//////////////////////////////////////

class Message
{
public:

    Message(int id, const std::string& msg, bool uplink)
        : msg_{ msg }
        , id_{ id }
        , uplink_{ uplink }
        , second_mux_{ false }
    {}

    void operator()(std::ostream& os = std::cout) const
    {
        os << "Message Id: " << id_ << (uplink_ ? ". Uplink message: " : ". Downlink message: " ) << msg_ << std::endl;
    }

    constexpr auto getId() const noexcept { return id_; }
    constexpr bool isUpLink() const noexcept { return uplink_; }
    constexpr bool isSecondMux() const noexcept { return second_mux_; }
    constexpr void setSecondMux() noexcept { second_mux_ = true; }

    void append(const std::string& s)
    {
        msg_ += "\n\t" + s;
    }

private:

    std::string msg_;
    int id_;
    bool uplink_;
    bool second_mux_;
};

//////////////////////////////////////
// Adaptors
//////////////////////////////////////

struct NetworkAdapter
{
    constexpr auto accept(auto&& msg)
    {
        auto& emsg = msg.get();
        emsg.append( "NetworkAdapter" );
        return emsg.isUpLink() ? reconduits::NextSide::a : reconduits::NextSide::done;
    }
};

struct ApplicationAdapter
{
    constexpr auto accept(auto&& msg)
    {
        auto& emsg = msg.get();
        emsg.append( "ApplicationAdapter" );
        return ! emsg.isUpLink() ? reconduits::NextSide::a : reconduits::NextSide::done;
    }
};

//////////////////////////////////////
// Factories
//////////////////////////////////////

class TCPConnectionFactory
{
public:

    constexpr reconduits::Conduit* accept(auto&& msg, reconduits::Conduit* a, reconduits::Conduit* b)
    {
        using T = std::decay_t<decltype(msg)>;
        if      constexpr (std::is_same_v<T, reconduits::Setup<Message>>) return create(msg, a, b);
        else if constexpr (std::is_same_v<T, reconduits::Release<Message>>) clean(msg, a, b);
        return nullptr;
    }

private:

    reconduits::Conduit* create(reconduits::Setup<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b);
    void clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b);
};

//////////////////////////////////////
// Muxes
//////////////////////////////////////

class ConnectionsMux
{
public:

    using key_type   = int;
    using value_type = reconduits::Conduit*;

    constexpr auto accept(auto&& msg)
    {
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{:p}] accepts a new message.", static_cast<void*>(this));
        auto& emsg = msg.get();
        emsg.append( "ConnectionsMux" );
        if( emsg.isSecondMux() ) return reconduits::NextSide::a;
        emsg.setSecondMux();
        return reconduits::NextSide::b;
    }

    auto find(auto&& msg) const
    {
        auto it = mux_table_.find( msg.get().getId() );
        bool found = it != mux_table_.cend();
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{0:p}] has{1}found key {2}.", static_cast<const void*>(this), (found ? " ":" NOT "), msg.get().getId());
        return std::pair{(found ? it->second : nullptr), found};
    }

    auto insert(auto&& key, reconduits::Conduit& c)
    {
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{:p}] is going to connect new conduits.", static_cast<void*>(this));
        auto [it, inserted] = mux_table_.emplace(key, &c);
        if( inserted ) return it->second;
        else {
            getLogger()->warn("The new connection for key {} was not possible.", key);
            return static_cast<reconduits::Conduit*>( nullptr );
        }
    }

    auto create(reconduits::Conduit* conduit_origin, auto&& msg) const
    {
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{:p}] is going to create new conduits.", static_cast<const void*>(this));
        auto& emsg = msg.get();
        return reconduits::Setup<reconduits::embedded_t<decltype(msg)>>{emsg, conduit_origin};
    }

    auto erase(auto&& key)
    {
        SPDLOG_DEBUG(getLogger(), "ConnectionsMux [{:p}] is going to release connected conduits.", static_cast<void*>(this));
        auto it = mux_table_.find( key );
        if( it != mux_table_.cend() ) {
            mux_table_.erase( it );
            return it->second;
        } else
            return static_cast<reconduits::Conduit*>( nullptr );
    }

protected:

    using mux_table_type = std::unordered_map<key_type, reconduits::Conduit*>;
    mux_table_type mux_table_;
};

class ConnectionsLUAMux : public ConnectionsMux
{
public:
    constexpr auto accept(auto&& msg)
    {
        SPDLOG_DEBUG(getLogger(), "ConnectionsLUAMux [{:p}] accepts a new message.", static_cast<void*>(this));
        auto& emsg = msg.get();
        lua.set_function("getId", [ & ]{ emsg.append( "ConnectionsLUAMux" ); });
        lua.script("getId()");
        if( emsg.isSecondMux() ) return reconduits::NextSide::a;
        emsg.setSecondMux();
        return reconduits::NextSide::b;
    }

private:
    sol::state lua;
};

//////////////////////////////////////
// Protocols
//////////////////////////////////////

struct IPProtocol
{
    constexpr auto accept(auto&& msg)
    {
        auto& emsg = msg.get();
        emsg.append( "IPProtocol" );
        return std::pair{ (emsg.isUpLink() ? reconduits::NextSide::b : reconduits::NextSide::a), &msg };
    }
};

struct TCPProtocol
{
    constexpr auto accept(auto&& msg)
    {
        auto& emsg = msg.get();
        emsg.append( "TCPProtocol" );
        return std::pair{ (emsg.isUpLink() ? reconduits::NextSide::b : reconduits::NextSide::a), &msg };
    }
};

struct HTTPProtocol
{
    constexpr auto accept(auto&& msg)
    {
        auto& emsg = msg.get();
        emsg.append( "HTTPProtocol" );
        return std::pair{ (emsg.isUpLink() ? reconduits::NextSide::b : reconduits::NextSide::a), &msg };
    }
};

//////////////////////////////////////
// Conduit Types
//////////////////////////////////////

GENERATE_ADAPTER_CONDUITS(  NetworkAdapter, ApplicationAdapter    );
GENERATE_FACTORY_CONDUITS(  TCPConnectionFactory                  );
GENERATE_MUX_CONDUITS(      ConnectionsMux, ConnectionsLUAMux     );
GENERATE_PROTOCOL_CONDUITS( IPProtocol, TCPProtocol, HTTPProtocol );

#include "ConduitTypes.hpp"

//////////////////////////////////////
// Inline definitions
//////////////////////////////////////

inline reconduits::Conduit* TCPConnectionFactory::create(reconduits::Setup<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    using namespace reconduits;
    auto& p2 = *new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ TCPProtocol{} } };
    auto& p3 = *new ( getFromPool<sizeof(Conduit)>() ) Conduit{ Protocol{ HTTPProtocol{} } };

    p2.setSideA( *a );
    p2.setSideB( p3 );

    p3.setSideA( p2 );
    p3.setSideB( *b );

    auto& emsg = msg.get();
    emsg.append( "TCPConnectionFactory: Setup" );

    auto key = emsg.getId();
    if( emsg.isUpLink() )
        return a->insertInSideB(key, p2) && b->insertInSideB(key, p3) ? &p2 : nullptr;
    else
        return a->insertInSideB(key, p2) && b->insertInSideB(key, p3) ? &p3 : nullptr;
}

void TCPConnectionFactory::clean(reconduits::Release<Message>& msg, reconduits::Conduit* a, reconduits::Conduit* b)
{
    auto& emsg = msg.get();
    emsg.append( "TCPConnectionFactory: Release" );
    auto key = emsg.getId();
    using namespace reconduits;
    putToPool<sizeof(Conduit)>( a->eraseFromSideB( key ) );
    putToPool<sizeof(Conduit)>( b->eraseFromSideB( key ) );
}

//////////////////////////////////////
// Tests
//////////////////////////////////////

TEST(ConduitTest, SimpleMessages) {

    using namespace std;
    using namespace reconduits;

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

    const char* request_msg = {
        "\tNetworkAdapter\n"
        "\tIPProtocol\n"
        "\tConnectionsLUAMux\n"
        "\tTCPConnectionFactory: Setup\n"
        "\tTCPProtocol\n"
        "\tHTTPProtocol\n"
        "\tConnectionsMux\n"
        "\tApplicationAdapter\n"
    };

    const char* response_msg = {
        "\tApplicationAdapter\n"
        "\tConnectionsMux\n"
        "\tHTTPProtocol\n"
        "\tTCPProtocol\n"
        "\tConnectionsLUAMux\n"
        "\tIPProtocol\n"
        "\tNetworkAdapter\n"
    };

    for( auto msg_cnt = 0; msg_cnt < 10; ++msg_cnt) {
        stringstream ss_req, ss_conduit_req;
        Message greeting{msg_cnt, "Hello word!!!", true};
        logger->info( " -------------- {} ------------------ ", msg_cnt );
        a1.accept( InformationChunk<Message>{ greeting } );
        greeting( ss_conduit_req );
        ss_req << "Message Id: " << msg_cnt << ". Uplink message: Hello word!!!\n" << request_msg;
        EXPECT_EQ(ss_req.str(), ss_conduit_req.str());

        stringstream ss_res, ss_conduit_res;
        Message response{msg_cnt, "Hi There!!!", false};
        logger->info( " -------------- {} ------------------ ", msg_cnt );
        a2.accept( InformationChunk<Message>{ response } );
        response( ss_conduit_res );
        ss_res << "Message Id: " << msg_cnt << ". Downlink message: Hi There!!!\n" << response_msg;
        EXPECT_EQ(ss_res.str(), ss_conduit_res.str());
    }
}
