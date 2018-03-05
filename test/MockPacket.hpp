#pragma once

#include <variant>
#include <cstdint>
#include <tuple>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


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
public:
    TCPHeader(uint16_t src, uint16_t dst)
        : src_{ htons( src ) }
        , dst_{ htons( dst ) }
    {}

    auto get_src_port() const { return src_; }
    auto get_dst_port() const { return dst_; }

private:
    uint16_t src_;
    uint16_t dst_;
};

class HTTPHeader
{
public:
private:
};

class TLSHeader
{
public:
private:
};

class Packet
{
public:
    using network_type     = std::variant<IPv4Header, IPv6Header>;
    using transport_type   = std::variant<UDPHeader, TCPHeader>;
    using application_type = std::variant<HTTPHeader, TLSHeader>;

    Packet(const auto& network, const auto& transport, const auto& app)
        : network_{ network }
        , transport_{ transport }
        , application_{ app }
    {}

    Packet(auto&& network, auto&& transport, auto&& app)
        : network_{ std::move( network ) }
        , transport_{ std::move( transport ) }
        , application_{ std::move( app ) }
    {}

    auto get_proto() const
    {
        ProtocolType proto; 
        auto get_proto_info = [ &proto ](auto&& header) { proto = header.get_proto(); };
        dispatch(network_, get_proto_info);
        return proto;
    }

    auto get_src_addr() const
    {
        in_addr_t addr; 
        auto get_addr_info = [ &addr ](auto&& header) { addr = header.get_src_addr(); };
        dispatch(network_, get_addr_info);
        return addr;
    }

    auto get_dst_addr() const
    {
        in_addr_t addr; 
        auto get_addr_info = [ &addr ](auto&& header) { addr = header.get_dst_addr(); };
        dispatch(network_, get_addr_info);
        return addr;
    }

    auto get_src_port() const
    {
        uint16_t port;
        auto get_port_info = [ &port ](auto&& header) { port = header.get_src_port(); };
        dispatch(transport_, get_port_info);
        return port;
    }

    auto get_dst_port() const
    {
        uint16_t port;
        auto get_port_info = [ &port ](auto&& header) { port = header.get_dst_port(); };
        dispatch(transport_, get_port_info);
        return port;
    }

private:
    template<typename V>
    constexpr void dispatch(V v, auto&& f) const { std::visit([ &f ](auto&& p) { f( p ); }, v); }

    network_type     network_;
    transport_type   transport_;
    application_type application_;
};

}
