#include "Connection.hpp"
#include <arpa/inet.h>
#include <iostream>

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(7033);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr); // ou slow.gmelodie.com

    std::array<uint8_t, 16> sid;
    uint32_t sttl = 0, seq = 0;

    if (Connection::threeWayHandshake(sockfd, server, sid, sttl, seq)) {
        std::cout << "[OK] Handshake aceito. SID: ";
        for (auto b : sid) std::cout << std::hex << (int)b;
        std::cout << "\nTTL: " << sttl << " Seq: " << seq << std::endl;
        return 0;
    } else {
        std::cerr << "[ERRO] Handshake falhou" << std::endl;
        return 1;
    }
}