#include "Serializer.hpp"   // Declarações da classe Serializer e da estrutura SLOWPacket
#include <cstring>          // Para uso de std::memcpy

// Serializa um objeto SLOWPacket em um vetor de bytes
std::vector<uint8_t> Serializer::serialize(const SLOWPacket& pkt) {
    // Aloca buffer com tamanho fixo de 32 bytes + tamanho dos dados (payload)
    std::vector<uint8_t> buffer(32 + pkt.data.size());
    size_t offset = 0;

    // 1) SID (16 bytes)
    std::memcpy(&buffer[offset], pkt.sid.data(), 16);
    offset += 16;

    //2)
    // Combina sttl e flags
    // Combina sttl e flags como os últimos 5 bits
    uint32_t sttlAndFlags = (pkt.sttl << 5) | (pkt.flags & 0x1F);

    // Serializa sttlAndFlags em little-endian (LSB primeiro)
    buffer[offset++] = static_cast<uint8_t>(sttlAndFlags & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((sttlAndFlags >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((sttlAndFlags >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((sttlAndFlags >> 24) & 0xFF);

    // 3) seqnum (4 bytes little endian)
    buffer[offset++] = static_cast<uint8_t>(pkt.seqnum & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((pkt.seqnum >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((pkt.seqnum >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((pkt.seqnum >> 24) & 0xFF);

    // 4) acknum (4 bytes little endian)
    buffer[offset++] = static_cast<uint8_t>(pkt.acknum & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((pkt.acknum >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((pkt.acknum >> 16) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>((pkt.acknum >> 24) & 0xFF);

    // 5) window (2 bytes big endian — como observado na imagem)
    buffer[offset++] = static_cast<uint8_t>((pkt.window >> 8) & 0xFF);
    buffer[offset++] = static_cast<uint8_t>(pkt.window & 0xFF);

    // 6) fid (1 byte)
    buffer[offset++] = pkt.fid;

    // 7) fo (1 byte)
    buffer[offset++] = pkt.fo;

    // 8) data (0–1440 bytes)
    if (!pkt.data.empty()) {
        std::memcpy(&buffer[offset], pkt.data.data(), pkt.data.size());
    }

    return buffer;
}

// Desserializa um vetor de bytes em um objeto SLOWPacket
SLOWPacket Serializer::deserialize(const std::vector<uint8_t>& buffer) {
    SLOWPacket pkt;      // Pacote a ser preenchido
    size_t offset = 0;

    // Verifica se o buffer tem o tamanho mínimo necessário (32 bytes)
    if (buffer.size() < 32) {
        return pkt;
    }
    // 1) Copia os 16 bytes do SID
    std::memcpy(pkt.sid.data(), &buffer[offset], 16);
    offset += 16;

    // 2) Lê sttlAndFlags como um inteiro de 32 bits (little-endian)
    uint32_t sttlAndFlags = 0;
    sttlAndFlags |= static_cast<uint32_t>(buffer[offset++]);
    sttlAndFlags |= static_cast<uint32_t>(buffer[offset++]) << 8;
    sttlAndFlags |= static_cast<uint32_t>(buffer[offset++]) << 16;
    sttlAndFlags |= static_cast<uint32_t>(buffer[offset++]) << 24;

    // Separa sttl (27 bits) e flags (5 bits)
    pkt.sttl = sttlAndFlags & 0xFFFFFFE0;   // Zera os últimos 5 bits
    pkt.flags = sttlAndFlags & 0x1F;        // Mantém apenas os 5 bits menos significativos

     // 3) Lê seqnum (4 bytes, little-endian)
    uint32_t seqnum_val = 0;
    seqnum_val |= static_cast<uint32_t>(buffer[offset++]);
    seqnum_val |= static_cast<uint32_t>(buffer[offset++]) << 8;
    seqnum_val |= static_cast<uint32_t>(buffer[offset++]) << 16;
    seqnum_val |= static_cast<uint32_t>(buffer[offset++]) << 24;
    pkt.seqnum = seqnum_val;

    // 4) Lê acknum (4 bytes, little-endian)
    uint32_t acknum_val = 0;
    acknum_val |= static_cast<uint32_t>(buffer[offset++]);
    acknum_val |= static_cast<uint32_t>(buffer[offset++]) << 8;
    acknum_val |= static_cast<uint32_t>(buffer[offset++]) << 16;
    acknum_val |= static_cast<uint32_t>(buffer[offset++]) << 24;
    pkt.acknum = acknum_val;

     // 5) Lê window (2 bytes, big-endian)
    uint16_t window_val = 0;
    window_val |= static_cast<uint16_t>(buffer[offset++]) << 8;
    window_val |= static_cast<uint16_t>(buffer[offset++]);
    pkt.window = window_val;

     // 6) Lê fid (1 byte)
    pkt.fid = buffer[offset++];
    // 7) Lê fo (1 byte)
    pkt.fo = buffer[offset++];

    // 8) Copia o restante do buffer para o campo de dados
    if (buffer.size() > offset) {
        pkt.data.assign(buffer.begin() + offset, buffer.end());
    }
    
    // Retorna o pacote preenchido
    return pkt;
}
