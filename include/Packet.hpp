// ===== include/Packet.hpp =====
#ifndef PACKET_HPP
#define PACKET_HPP

#include <array>
#include <vector>
#include <cstdint>
#include <cstddef>

constexpr size_t MAX_DATA_SIZE = 1440;

// Flags do protocolo SLOW (5 bits)
enum SLOWFlags : uint8_t
{
    MB = 1 << 0,      // 0x01
    ACK = 1 << 1,     // 0x02
    REVIVE = 1 << 3,  // 0x08
    CONNECT = 1 << 4, // ⚠️ 0x10   <- Correto
    ACCEPT = CONNECT  // mesmo valor
};

struct SLOWPacket
{
    std::array<uint8_t, 16> sid{}; // 16 bytes de sessão (UUID)
    uint32_t sttl;
    uint8_t flags;             // Flags do pacote (5 bits)
    uint32_t seqnum;           // Número de sequência (4 bytes)
    uint32_t acknum;           // Número de confirmação (4 bytes)
    uint16_t window;           // Janela de fluxo (2 bytes)
    uint8_t fid;               // Fragment ID (1 byte)
    uint8_t fo;                // Fragment offset (1 byte)
    std::vector<uint8_t> data; // Payload de dados (0 a 1440 bytes)

    // Construtor padrão inicializa tudo com zero
    SLOWPacket()
        : sttl(0), flags(0), seqnum(0), acknum(0), window(0), fid(0), fo(0), data() {}
};

#endif // PACKET_HPP
