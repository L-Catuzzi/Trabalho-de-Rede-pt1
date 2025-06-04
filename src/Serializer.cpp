#include "Serializer.hpp"
#include <cstring>

std::vector<uint8_t> Serializer::serialize(const SLOWPacket& pkt) {
    std::vector<uint8_t> buffer(32 + pkt.data.size());  // header + data
    size_t offset = 0;

    std::memcpy(&buffer[offset], pkt.sid.data(), 16); offset += 16;

    uint32_t sttl_flags = (pkt.sttl << 5) | (pkt.flags & 0x1F);
    std::memcpy(&buffer[offset], &sttl_flags, 4); offset += 4;

    std::memcpy(&buffer[offset], &pkt.seqnum, 4); offset += 4;
    std::memcpy(&buffer[offset], &pkt.acknum, 4); offset += 4;
    std::memcpy(&buffer[offset], &pkt.window, 2); offset += 2;
    buffer[offset++] = pkt.fid;
    buffer[offset++] = pkt.fo;

    std::memcpy(&buffer[offset], pkt.data.data(), pkt.data.size());

    return buffer;
}

SLOWPacket Serializer::deserialize(const std::vector<uint8_t>& buffer) {
    SLOWPacket pkt;
    size_t offset = 0;

    std::memcpy(pkt.sid.data(), &buffer[offset], 16); offset += 16;

    uint32_t sttl_flags;
    std::memcpy(&sttl_flags, &buffer[offset], 4); offset += 4;
    pkt.sttl = sttl_flags >> 5;
    pkt.flags = sttl_flags & 0x1F;

    std::memcpy(&pkt.seqnum, &buffer[offset], 4); offset += 4;
    std::memcpy(&pkt.acknum, &buffer[offset], 4); offset += 4;
    std::memcpy(&pkt.window, &buffer[offset], 2); offset += 2;
    pkt.fid = buffer[offset++];
    pkt.fo = buffer[offset++];

    pkt.data.resize(buffer.size() - offset);
    std::memcpy(pkt.data.data(), &buffer[offset], pkt.data.size());

    return pkt;
}
