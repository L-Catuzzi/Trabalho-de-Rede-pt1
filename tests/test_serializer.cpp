#include "Packet.hpp"   // Define a estrutura SLOWPacket e flags como ACK
#include "Serializer.hpp"   // Contém funções serialize() e deserialize()
#include <iostream>          // Para saída padrão (std::cout, std::cerr)

int main() {
     // 1) Cria e preenche um pacote SLOWPacket de teste
    SLOWPacket pkt;
    pkt.seqnum = 42;    // Número de sequência
    pkt.flags = ACK;    // Define a flag ACK
    pkt.window = 4096;  // Tamanho da janela
    pkt.data = std::vector<uint8_t>{'H', 'e', 'l', 'l', 'o'};   // Payload com "Hello"

     // 2) Serializa o pacote em um vetor de bytes
    auto buffer = Serializer::serialize(pkt);
     // 3) Desserializa os bytes de volta em um novo pacote
    auto pkt2 = Serializer::deserialize(buffer);

    // 4) Verifica se os dados essenciais foram preservados
    if (pkt2.seqnum == 42 && pkt2.window == 4096 && pkt2.data == pkt.data) {    // Verifica número de sequência, Verifica tamanho da janela e // Compara conteúdo do payload
        std::cout << "[OK] Serialização e desserialização funcionam\n";
        return 0; // Sucesso
    } else {
        std::cerr << "[ERRO] Dados divergentes após desserialização\n";
        return 1; // Erro
    }
}