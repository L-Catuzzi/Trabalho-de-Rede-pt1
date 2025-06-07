#include "Serializer.hpp"
#include <cstring>

//Transforma um objeto SLOWPacket em um std::vector<uint8_t> para ser enviado via socket.
std::vector<uint8_t> Serializer::serialize(const SLOWPacket& pkt) {
    // O cabeçalho SLOW tem 32 bytes + tamanho de data
    std::vector<uint8_t> buffer(32 + pkt.data.size());
    size_t offset = 0;

    // 1) SID: 16 bytes (0..15)
    std::memcpy(&buffer[offset], pkt.sid.data(), 16);
    offset += 16;

    // 2) sttl+flags: 4 bytes (16..19), em LITTLE-ENDIAN
    uint32_t sttl_flags_val = (static_cast<uint32_t>(pkt.flags) << 27) | pkt.sttl;
    buffer[offset++] = static_cast<uint8_t>(sttl_flags_val & 0xFF);         // LSB
    buffer[offset++] = static_cast<uint8_t>((sttl_flags_val >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((sttl_flags_val >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((sttl_flags_val >> 24) & 0xFF); // MSB

    // 3) seqnum: 4 bytes (20..23), em LITTLE-ENDIAN
    uint32_t seqnum_val = pkt.seqnum;
    buffer[offset++] = static_cast<uint8_t>(seqnum_val & 0xFF);         // LSB
    buffer[offset++] = static_cast<uint8_t>((seqnum_val >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((seqnum_val >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((seqnum_val >> 24) & 0xFF); // MSB

    // 4) acknum: 4 bytes (24..27), em LITTLE-ENDIAN
    uint32_t acknum_val = pkt.acknum;
    buffer[offset++] = static_cast<uint8_t>(acknum_val & 0xFF);         // LSB
    buffer[offset++] = static_cast<uint8_t>((acknum_val >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((acknum_val >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((acknum_val >> 24) & 0xFF); // MSB

    // 5) window: 2 bytes (28..29), em LITTLE-ENDIAN
    uint16_t window_val = pkt.window;
    buffer[offset++] = static_cast<uint8_t>(window_val & 0xFF);        // LSB
    buffer[offset++] = static_cast<uint8_t>((window_val >> 8) & 0xFF); // MSB

    // 6) fid: 1 byte (30)
    buffer[offset++] = pkt.fid;

    // 7) fo: 1 byte (31)
    buffer[offset++] = pkt.fo;

    // 8) data: (32..fim)
    if (!pkt.data.empty()) {
        std::memcpy(&buffer[offset], pkt.data.data(), pkt.data.size());
    }

    return buffer;
}

//Constrói um objeto SLOWPacket a partir dos bytes recebidos pela rede.
SLOWPacket Serializer::deserialize(const std::vector<uint8_t>& buffer) {
    SLOWPacket pkt;
    size_t offset = 0;

    // Validação básica do tamanho do buffer (cabeçalho mínimo de 32 bytes) -Implementar futuramente (??)
    if (buffer.size() < 32) {
        // Tratar erro: buffer muito pequeno para ser um pacote SLOW válido
        // Lançar uma exceção ou retornar um pacote inválido, dependendo da sua estratégia de erro.
        return pkt;
    }

    // 1) SID: 16 bytes (0..15)
    std::memcpy(pkt.sid.data(), &buffer[offset], 16);
    offset += 16;

    // 2) sttl+flags: 4 bytes (16..19) de LITTLE-ENDIAN para host
    uint32_t sttl_flags_val = 0;
    sttl_flags_val |= static_cast<uint32_t>(buffer[offset++]);
    sttl_flags_val |= static_cast<uint32_t>(buffer[offset++]) << 8;
    sttl_flags_val |= static_cast<uint32_t>(buffer[offset++]) << 16;
    sttl_flags_val |= static_cast<uint32_t>(buffer[offset++]) << 24;
    
    pkt.flags = (sttl_flags_val >> 27) & 0x1F; // Extract 5 bits for flags
    pkt.sttl  = sttl_flags_val & 0x07FFFFFF;

    // 3) seqnum: 4 bytes (20..23) de LITTLE-ENDIAN para host
    uint32_t seqnum_val = 0;
    seqnum_val |= static_cast<uint32_t>(buffer[offset++]);
    seqnum_val |= static_cast<uint32_t>(buffer[offset++]) << 8;
    seqnum_val |= static_cast<uint32_t>(buffer[offset++]) << 16;
    seqnum_val |= static_cast<uint32_t>(buffer[offset++]) << 24;
    pkt.seqnum = seqnum_val;

    // 4) acknum: 4 bytes (24..27) de LITTLE-ENDIAN para host
    uint32_t acknum_val = 0;
    acknum_val |= static_cast<uint32_t>(buffer[offset++]);
    acknum_val |= static_cast<uint32_t>(buffer[offset++]) << 8;
    acknum_val |= static_cast<uint32_t>(buffer[offset++]) << 16;
    acknum_val |= static_cast<uint32_t>(buffer[offset++]) << 24;
    pkt.acknum = acknum_val;

    // 5) window: 2 bytes (28..29) de LITTLE-ENDIAN para host
    uint16_t window_val = 0;
    window_val |= static_cast<uint16_t>(buffer[offset++]);
    window_val |= static_cast<uint16_t>(buffer[offset++]) << 8;
    pkt.window = window_val;

    // 6) fid: 1 byte (30)
    pkt.fid = buffer[offset++];

    // 7) fo: 1 byte (31)
    pkt.fo = buffer[offset++];

    // 8) data: (32..fim)
    if (buffer.size() > offset) {
        pkt.data.assign(buffer.begin() + offset, buffer.end());
    }

    return pkt;
}