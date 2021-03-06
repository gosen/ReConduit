#pragma once

#include <variant>
#include <type_traits>
#include <cstdint>
#include <tuple>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DISPACHER_INVOKER(VARIANT, FUNC, ARGS...) \
    std::visit([](auto&& header) { return header.FUNC( ARGS ); }, VARIANT)

namespace mock_packet {

enum class ProtocolType { tcp = 6, udp = 17 };

class IPv4Header
{
public:
    IPv4Header(const char* src, const char* dst, ProtocolType proto)
        : src_{ inet_network( src ) }
        , dst_{ inet_network( dst ) }
        , proto_{ proto }
    {}

    auto get_src_addr() const noexcept { return src_; }
    auto get_dst_addr() const noexcept { return dst_; }
    auto get_proto() const noexcept { return proto_; }

private:
    in_addr_t src_;
    in_addr_t dst_;
    ProtocolType  proto_;
};

class IPv6Header
{
public:
    IPv6Header(const char* src, const char* dst, ProtocolType proto)
        : src_{ convert_2_number( src ) }
        , dst_{ convert_2_number( dst ) }
        , proto_{ proto }
    {}

    auto get_src_addr() const noexcept { return src_.s6_addr32[0]; }
    auto get_dst_addr() const noexcept { return dst_.s6_addr32[0]; }
    auto get_proto() const noexcept { return proto_; }

private:
    auto convert_2_number(const char* addr) const -> in6_addr
    {
        in6_addr in6;
        in6.s6_addr32[0] = addr[0]; // Wrong, but ok by now...
        in6.s6_addr32[1] = addr[1]; // Wrong, but ok by now...
        in6.s6_addr32[2] = addr[2]; // Wrong, but ok by now...
        in6.s6_addr32[3] = addr[3]; // Wrong, but ok by now...
        return in6;
    }

    in6_addr src_;
    in6_addr dst_;
    ProtocolType proto_;
};

class UDPHeader
{
public:
    UDPHeader(uint16_t src, uint16_t dst)
        : src_{ htons( src ) }
        , dst_{ htons( dst ) }
    {}

    auto get_src_port() const noexcept { return htons( src_ ); }
    auto get_dst_port() const noexcept { return htons( dst_ ); }

private:
    uint16_t src_;
    uint16_t dst_;
};

class TCPHeader
{
    using flags_type = std::tuple<bool, bool, bool, bool>;

public:

    TCPHeader(uint16_t src, uint16_t dst, flags_type f)
        : src_{ htons( src ) }
        , dst_{ htons( dst ) }
        , flags_{ f }
    {}

    auto get_src_port() const noexcept { return htons( src_ ); }
    auto get_dst_port() const noexcept { return htons( dst_ ); }

    constexpr bool is_ack()     const noexcept { return std::get<0>( flags_ ); }
    constexpr bool is_syn()     const noexcept { return std::get<1>( flags_ ); }
    constexpr bool is_fin()     const noexcept { return std::get<2>( flags_ ); }
    constexpr bool is_timeout() const noexcept { return std::get<3>( flags_ ); }

    static auto set_syn_flag()          { return std::tuple{false, true,  false, false}; }
    static auto set_ack_flag()          { return std::tuple{true,  false, false, false}; }
    static auto set_syn_ack_flags()     { return std::tuple{true,  true,  false, false}; }
    static auto set_fin_flag()          { return std::tuple{false, false, true,  false}; }
    static auto set_fin_ack_flags()     { return std::tuple{true,  false, true,  false}; }
    static auto set_ack_timeout_flags() { return std::tuple{true,  false, false, true }; }

private:
    uint16_t src_;
    uint16_t dst_;
    flags_type flags_;
};

class HTTPHeader
{
public:
    explicit HTTPHeader(std::string_view data)
        : data_{ data }
    {}

    const auto& get_url() const noexcept { return data_; }
    const auto& get_response_code() const noexcept { return data_; }

private:
    std::string data_;
};

class TLSHeader
{
public:
    explicit TLSHeader(std::string_view data)
        : data_{ data }
    {}

    const auto& get_server_name_idication() const noexcept { return data_; }
    const auto& get_application_layer_protocol_negociation() const noexcept { return data_; }

private:
    std::string data_;
};

class DNSHeader
{
public:
    explicit DNSHeader(std::string_view data)
        : data_{ data }
    {}

    const auto& get_uri() const noexcept { return data_; }

private:
    std::string data_;
};

class Packet
{
public:
    using network_type     = std::variant<IPv4Header, IPv6Header>;
    using transport_type   = std::variant<UDPHeader, TCPHeader>;
    using application_type = std::variant<std::monostate, HTTPHeader, TLSHeader, DNSHeader>;

    using l3_id_type = ProtocolType;
    using l4_id_type = std::tuple<uint32_t, uint32_t, uint16_t, uint16_t>;

    Packet(const auto& network, const auto& transport)
        : network_{ network }
        , transport_{ transport }
        , application_{}
    {}

    Packet(const auto& network, const auto& transport, const auto& app)
        : network_{ network }
        , transport_{ transport }
        , application_{ app }
    {}

    Packet(auto&& network, auto&& transport)
        : network_{ std::move( network ) }
        , transport_{ std::move( transport ) }
        , application_{}
    {}

    Packet(auto&& network, auto&& transport, auto&& app = application_type{})
        : network_{ std::move( network ) }
        , transport_{ std::move( transport ) }
        , application_{ std::move( app ) }
    {}

    constexpr auto get_proto()    const { return DISPACHER_INVOKER(network_, get_proto); }
    constexpr auto get_src_addr() const { return DISPACHER_INVOKER(network_, get_src_addr); }
    constexpr auto get_dst_addr() const { return DISPACHER_INVOKER(network_, get_dst_addr); }

    constexpr auto get_src_port() const { return DISPACHER_INVOKER(transport_, get_src_port); }
    constexpr auto get_dst_port() const { return DISPACHER_INVOKER(transport_, get_dst_port); }

    constexpr auto is_ack() const { return std::get<TCPHeader>( transport_ ).is_ack(); }
    constexpr auto is_syn() const { return std::get<TCPHeader>( transport_ ).is_syn(); }
    constexpr auto is_fin() const { return std::get<TCPHeader>( transport_ ).is_fin(); }

    constexpr auto is_timeout() const { return std::get<TCPHeader>( transport_ ).is_timeout(); }

    template<typename T>
    const auto& get_app_proto() const { return std::get<T>( application_ ); }

private:

    network_type     network_;
    transport_type   transport_;
    application_type application_;
};

}

inline std::ostream& operator<<(std::ostream& o, const mock_packet::Packet::l4_id_type& l4)
{
    return o << "["
        << std::get<0>( l4 ) << ", "
        << std::get<1>( l4 ) << ", "
        << std::get<2>( l4 ) << ", "
        << std::get<3>( l4 ) << "]";
}
