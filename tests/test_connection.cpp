#include "Connection.hpp"   // Declaração da classe Connection e threeWayHandshake
#include <arpa/inet.h>      // Funções de rede como inet_pton
#include <iostream>         // Entrada/saída padrão

int main() {
    // 1) Cria um socket UDP (AF_INET = IPv4, SOCK_DGRAM = UDP)
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // 2) Configura estrutura do servidor
    sockaddr_in server{};
    server.sin_family = AF_INET;    // Família IPv4
    server.sin_port = htons(7033);   // Porta 7033 (conversão para network byte order)
    
    // 3) Converte string IP para formato binário e armazena em server.sin_addr
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr); // ou slow.gmelodie.com
    
    // 4) Declara variáveis para armazenar dados do handshake
    std::array<uint8_t, 16> sid; // Session ID
    uint32_t sttl = 0, seq = 0; // TTL retornado e Número de sequência inicial

     // 5) Executa handshake com o servidor
    if (Connection::threeWayHandshake(sockfd, server, sid, sttl, seq)) {
        // Sucesso: exibe informações retornadas
        std::cout << "[OK] Handshake aceito. SID: ";
        for (auto b : sid) std::cout << std::hex << (int)b;     // Exibe SID como hex
        std::cout << "\nTTL: " << sttl << " Seq: " << seq << std::endl;
        return 0;       // Sucesso
    } else {
        // Falha no handshake
        std::cerr << "[ERRO] Handshake falhou" << std::endl;
        return 1;   // Erro
    }
}