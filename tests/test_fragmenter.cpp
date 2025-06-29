#include "Fragmenter.hpp"   // Inclui a lógica de fragmentação de payloads
#include <iostream>         // Para saída padrão (std::cout, std::cerr)

int main() {
    // 1) Cria um payload de 4000 bytes preenchido com o caractere 'X'
    std::vector<uint8_t> payload(4000, 'X');

    // 2) Inicializa o Session ID (sid) com 16 bytes zerado
    std::array<uint8_t, 16> sid = {0};

    // 3) Número de sequência inicial
    uint32_t seq = 1;

    // 4) Fragmenta o payload em pacotes SLOW (1440 bytes cada no máximo)
    auto frags = Fragmenter::fragmentPayload(sid, 30000, seq, 4096, payload);
    
    // 5) Exibe quantos fragmentos foram gerados
    std::cout << "Fragmentos gerados: " << frags.size() << std::endl;
     // 6) Valida se foram gerados pelo menos 3 fragmentos (4000 / 1440 > 2)
    //    e se o primeiro fragmento contém a flag MB (MultiBlock)
    if (frags.size() >= 3 && frags[0].flags & MB) {
        std::cout << "[OK] Fragmentação correta\n";
        return 0; // Sucesso
    } else {
        std::cerr << "[ERRO] Fragmentação incorreta\n";
        return 1;   // Erro
    }
}
