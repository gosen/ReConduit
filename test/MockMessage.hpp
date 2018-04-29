#pragma once

#include "MockPacket.hpp"
#include "MockLogger.hpp"

#include <chrono>
#include <iostream>

namespace mock_conduits {

class Message
{
public:

    Message(
        std::chrono::system_clock::time_point tp,
        const mock_packet::Packet& pkt,
        bool uplink)
        : packet_{ pkt }
        , time_stamp_{ tp }
        , uplink_{ uplink }
    {}

    constexpr auto getId() const noexcept { return id_; }
    constexpr bool isUpLink() const noexcept { return uplink_; }

    void append(const std::string& s)
    {
        msg_ += s + "\n";
    }

private:

    friend std::ostream& operator<<(std::ostream& o, const Message& m)
    {
        return o << "---- \n" << m.msg_ << "----\n";
    }

    std::string msg_;
    int id_;

    mock_packet::Packet packet_;
    std::chrono::system_clock::time_point time_stamp_;
    bool uplink_;
};

}
