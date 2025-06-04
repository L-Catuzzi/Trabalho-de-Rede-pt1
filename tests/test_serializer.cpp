#include "Packet.hpp"
#include "Serializer.hpp"
#include <iostream>

int main() {
    SLOWPacket pkt;
    pkt.seqnum = 42;
    pkt.flags = ACK;
    pkt.window = 4096;
    pkt.data = std::vector<uint8_t>{'H', 'e', 'l', 'l', 'o'};

    auto buffer = Serializer::serialize(pkt);
    auto pkt2 = Serializer::deserialize(buffer);

    if (pkt2.seqnum == 42 && pkt2.window == 4096 && pkt2.data == pkt.data) {
        std::cout << "[OK] Serialização e desserialização funcionam\n";
        return 0;
    } else {
        std::cerr << "[ERRO] Dados divergentes após desserialização\n";
        return 1;
    }
}