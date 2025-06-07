#include "Connection.hpp"
#include "Fragmenter.hpp"
#include "Serializer.hpp"
#include "UuidGenerator.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>

int main(int argc, char* argv[]) {
    std::string server_ip = "slow.gmelodie.com";
    int port = 7033;
    uint16_t window = 4096;
    uint32_t sttl = 30000;

    // Cria um Socket UDP
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    //Prepara a struct sockaddr_in com IP e porta do servidor
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // Resolver DNS do host
    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int err = getaddrinfo(server_ip.c_str(), nullptr, &hints, &res);
    if (err != 0 || res == nullptr) {
        std::cerr << "Erro ao resolver host " << server_ip << ": " << gai_strerror(err) << std::endl;
        return 1;
    }

    //Copia o IP resolvido para a struct do servidor e libera a memória da getaddrinfo
    memcpy(&server.sin_addr, &((sockaddr_in*)res->ai_addr)->sin_addr, sizeof(in_addr));
    freeaddrinfo(res);

    std::cout << "Iniciando conexão com " << server_ip << ":" << port << "...\n";

    std::array<uint8_t, 16> sid;
    uint32_t confirmed_sttl = 0;
    uint32_t seqnum = 0;

    // Faz o threeWayHandshake, Envia o pacote connect e aguarda resposta
    if (!Connection::threeWayHandshake(sockfd, server, sid, confirmed_sttl, seqnum)) {
        std::cerr << "[ERRO] Handshake falhou ou não houve resposta.\n";
        close(sockfd);
        return 1;
    }

    //Está dando erro no threeWayHandshake, a partir daqui não da pra saber se está funcionando :(

    std::cout << "[OK] Handshake aceito pelo servidor!\n"
              << "     SID (UUID): ";
    for (auto b : sid) std::cout << std::hex << (int)b;
    std::cout << "\n     TTL da sessão: " << std::dec << confirmed_sttl
              << "\n     Próximo seqnum:  " << seqnum << "\n";

    std::string mensagem = "Hello from SLOW peripheral!";
    std::vector<uint8_t> payload(mensagem.begin(), mensagem.end());

    auto pacotes = Fragmenter::fragmentPayload(sid, confirmed_sttl, seqnum, window, payload);

    std::cout << "Enviando payload em " << pacotes.size() << " fragmento(s) ...\n";
    for (auto& pkt : pacotes) {
        auto buffer = Serializer::serialize(pkt);
        ssize_t sent = sendto(sockfd, buffer.data(), buffer.size(), 0, (sockaddr*)&server, sizeof(server));
        if (sent < 0) {
            perror("sendto");
            close(sockfd);
            return 1;
        }
        std::cout << "." << std::flush;
    }

    std::cout << "\nPayload enviado. Programa encerrando.\n";
    close(sockfd);
    return 0;
}
