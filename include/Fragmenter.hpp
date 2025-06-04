#ifndef FRAGMENTER_HPP
#define FRAGMENTER_HPP

#include "Packet.hpp"
#include <vector>

class Fragmenter {
public:
    static std::vector<SLOWPacket> fragmentPayload(
        const std::array<uint8_t, 16>& sid,
        uint32_t sttl,
        uint32_t& seqStart,
        uint16_t window,
        const std::vector<uint8_t>& payload);
};

#endif