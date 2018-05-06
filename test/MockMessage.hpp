#pragma once

#include "MockPacket.hpp"
#include "MockLogger.hpp"

#include <utility>
#include <chrono>
#include <iostream>

namespace mock_conduits {

class Message
{
public:

    using key_type = std::variant<mock_packet::Packet::l3_id_type, mock_packet::Packet::l4_id_type>;

    Message(
        std::chrono::system_clock::time_point tp,
        const mock_packet::Packet& pkt,
        bool uplink)
        : packet_{ pkt }
        , time_stamp_{ tp }
        , uplink_{ uplink }
        , established_{}
    {}

    constexpr auto getL3Id() const noexcept { return key_type{ packet_.get_proto() }; }
    constexpr auto getL4Id() const noexcept
    {
        return key_type{ uplink_ ?
            std::tuple{packet_.get_src_addr(), packet_.get_dst_addr(), packet_.get_src_port(), packet_.get_dst_port()} :
            std::tuple{packet_.get_dst_addr(), packet_.get_src_addr(), packet_.get_dst_port(), packet_.get_src_port()} };
    }
    constexpr bool isUpLink() const noexcept { return uplink_; }

    const auto& packet() const noexcept { return packet_; }

    bool connection_established() const { return established_; }
    void set_connection_established() { established_ = true; }

    enum {
        unkonwn_app_protocol = 0,
        dns_app_protocol     = 53,
        http_app_protocol    = 80,
        tls_app_protocol     = 443,
    };

    auto app_proto() const noexcept
    {
        switch( ( isUpLink() ? packet_.get_dst_port() : packet_.get_src_port() ) ) {
            case dns_app_protocol:  return dns_app_protocol;
            case http_app_protocol: return http_app_protocol;
            case tls_app_protocol:  return tls_app_protocol;
            default: return unkonwn_app_protocol;
        }
    }

    void append(const std::string& s)
    {
        msg_ += s + "\n";
    }

private:

    friend std::ostream& operator<<(std::ostream& o, const Message& m)
    {
        return o << "Message transist:\n-----------\n" << m.msg_ << "\n\n";
    }

    std::string msg_;

    mock_packet::Packet packet_;
    std::chrono::system_clock::time_point time_stamp_;
    bool uplink_;
    bool established_;
};

}
