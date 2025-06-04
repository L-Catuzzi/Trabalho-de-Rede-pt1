#include "Fragmenter.hpp"
#include <iostream>

int main() {
    std::vector<uint8_t> payload(4000, 'X');
    std::array<uint8_t, 16> sid = {0};
    uint32_t seq = 1;

    auto frags = Fragmenter::fragmentPayload(sid, 30000, seq, 4096, payload);
    std::cout << "Fragmentos gerados: " << frags.size() << std::endl;

    if (frags.size() >= 3 && frags[0].flags & MB) {
        std::cout << "[OK] Fragmentação correta\n";
        return 0;
    } else {
        std::cerr << "[ERRO] Fragmentação incorreta\n";
        return 1;
    }
}
