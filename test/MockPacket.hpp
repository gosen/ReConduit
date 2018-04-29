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

#if 0
#define DISPACHER_INVOKER_FOR(TYPE, VARIANT, FUNC, ARGS...) \
    std::visit([](auto&& header) { \
        using T = std::decay_t<decltype(header)>; \
        if constexpr ( std::is_same_v<T, TYPE> ) return header.FUNC( ARGS ); }, VARIANT);
#endif

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

    auto get_src_addr() const { return src_; }
    auto get_dst_addr() const { return dst_; }
    auto get_proto() const { return proto_; }

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

    auto get_src_addr() const { return src_.s6_addr32[0]; }
    auto get_dst_addr() const { return dst_.s6_addr32[0]; }
    auto get_proto() const { return proto_; }

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

    auto get_src_port() const { return src_; }
    auto get_dst_port() const { return dst_; }

private:
    uint16_t src_;
    uint16_t dst_;
};

class TCPHeader
{
    using flags_type = std::tuple<bool, bool, bool>;

public:

    TCPHeader(uint16_t src, uint16_t dst, flags_type f)
        : src_{ htons( src ) }
        , dst_{ htons( dst ) }
        , flags_{ f }
    {}

    auto get_src_port() const { return src_; }
    auto get_dst_port() const { return dst_; }

    bool is_ack() const { return std::get<0>( flags_ ); }
    bool is_syn() const { return std::get<1>( flags_ ); }
    bool is_fin() const { return std::get<2>( flags_ ); }

    static auto set_syn_flag()      { return std::tuple{false, true, false}; }
    static auto set_ack_flag()      { return std::tuple{true,  false, false}; }
    static auto set_syn_ack_flags() { return std::tuple{true,  true,  false}; }
    static auto set_fin_ack_flags() { return std::tuple{true,  false, true}; }

private:
    uint16_t src_;
    uint16_t dst_;
    flags_type flags_;
};

class HTTPHeader
{
public:
    explicit HTTPHeader(std::string_view url)
        : url_{ url }
    {}

    auto get_url() const  { return url_; }

private:
    std::string url_;
};

class TLSHeader
{
public:
    explicit TLSHeader(std::string_view server_name_idication)
        : sni_{ server_name_idication }
    {}

    const auto& get_server_name_idication() const  { return sni_; }

private:
    std::string sni_;
};

class Packet
{
public:
    using network_type     = std::variant<IPv4Header, IPv6Header>;
    using transport_type   = std::variant<UDPHeader, TCPHeader>;
    using application_type = std::variant<std::monostate, HTTPHeader, TLSHeader>;

    Packet(const auto& network, const auto& transport, const auto& app = application_type{})
        : network_{ network }
        , transport_{ transport }
        , application_{ app }
    {}

    Packet(auto&& network, auto&& transport, auto&& app = application_type{})
        : network_{ std::move( network ) }
        , transport_{ std::move( transport ) }
        , application_{ std::move( app ) }
    {}

    auto get_proto()    const { return DISPACHER_INVOKER(network_, get_proto); }
    auto get_src_addr() const { return DISPACHER_INVOKER(network_, get_src_addr); }
    auto get_dst_addr() const { return DISPACHER_INVOKER(network_, get_dst_addr); }

    auto get_src_port() const { return DISPACHER_INVOKER(transport_, get_src_port); }
    auto get_dst_port() const { return DISPACHER_INVOKER(transport_, get_dst_port); }

    template<typename T>
    const auto& get_app_proto() const { return std::get<T>( application_ ); }

private:

//    auto dispacher(auto&& v, auto&& func, auto&&... args) { 
//        return std::visit([](auto&& header) { return header.func( std::forward< decltype(args)>( args)... ); }, v)
//    }

    network_type     network_;
    transport_type   transport_;
    application_type application_;
};

}
