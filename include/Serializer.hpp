
#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include "Packet.hpp"
#include <vector>

class Serializer {
public:
    static std::vector<uint8_t> serialize(const SLOWPacket& pkt);
    static SLOWPacket deserialize(const std::vector<uint8_t>& buffer);
};

#endif
