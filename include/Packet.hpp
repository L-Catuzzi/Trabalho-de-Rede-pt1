// include/Packet.hpp
#ifndef PACKET_HPP
#define PACKET_HPP

#include <array>
#include <vector>
#include <cstdint>

constexpr size_t MAX_DATA_SIZE = 1440;

// Aqui colocamos CONNECT e ACCEPT usando o mesmo valor (bit 2 = 0x04)
enum SLOWFlags : uint8_t {
    CONNECT       = 1 << 0,  // 0x01
    REVIVE        = 1 << 1,  // 0x02
    ACK           = 1 << 2,  // 0x04
    ACCEPT = 1 << 3,  // 0x08 (Example, for A/R flag from server)
    MB            = 1 << 4   // 0x10
};

struct SLOWPacket {
    std::array<uint8_t, 16> sid{};
    uint32_t sttl : 27;
    uint8_t  flags : 5;
    uint32_t seqnum;
    uint32_t acknum;
    uint16_t window;
    uint8_t  fid;
    uint8_t  fo;
    std::vector<uint8_t> data;

    SLOWPacket()
      : sttl(0),
        flags(0),
        seqnum(0),
        acknum(0),
        window(0),
        fid(0),
        fo(0),
        data() {}
};

#endif
