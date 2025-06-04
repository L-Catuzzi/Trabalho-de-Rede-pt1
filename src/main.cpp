#include "Connection.hpp"
#include "Fragmenter.hpp"
#include "Serializer.hpp"
#include "UuidGenerator.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    // Defaults
    std::string server_ip = "slow.gmelodie.com";
    int port = 7033;
    uint16_t window = 4096;
    uint32_t sttl = 30000;

    // Socket setup
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server.sin_addr) <= 0) {
        std::cerr << "Erro ao resolver IP de " << server_ip << std::endl;
        return 1;
    }

    std::cout << "Iniciando conexão com " << server_ip << ":" << port << "...\n";

    std::array<uint8_t, 16> sid;
    uint32_t seqnum = 0;
    uint32_t confirmed_sttl = 0;

    if (!Connection::threeWayHandshake(sockfd, server, sid, confirmed_sttl, seqnum)) {
        std::cerr << "Handshake falhou.\n";
        return 1;
    }

    std::cout << "Handshake OK. Enviando payload...\n";

    std::string mensagem = "Hello from SLOW peripheral!";
    std::vector<uint8_t> payload(mensagem.begin(), mensagem.end());

    auto pacotes = Fragmenter::fragmentPayload(sid, confirmed_sttl, seqnum, window, payload);
    for (auto& pkt : pacotes) {
        auto buffer = Serializer::serialize(pkt);
        sendto(sockfd, buffer.data(), buffer.size(), 0, (sockaddr*)&server, sizeof(server));
        std::cout << "." << std::flush;
    }

    std::cout << "\nPayload enviado. Finalizando.\n";
    close(sockfd);
    return 0;
}