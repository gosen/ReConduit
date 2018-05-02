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
    constexpr auto getL4Id() const noexcept { return key_type{  uplink_ ? packet_.get_src_port() : packet_.get_dst_port() }; }
    constexpr bool isUpLink() const noexcept { return uplink_; }
    
    const auto& packet() const { return packet_; }     

    bool connection_established() const { return established_; }
    void set_connection_established() { established_ = true; }

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
