#include "Packet.hpp"
#include "Serializer.hpp"

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7033);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    SLOWPacket pkt;
    pkt.flags = CONNECT;
    pkt.sttl = 30000;
    pkt.seqnum = 1;
    pkt.window = 4096;
    pkt.data = std::vector<uint8_t>{'H','e','l','l','o'};

    auto serialized = Serializer::serialize(pkt);
    sendto(sockfd, serialized.data(), serialized.size(), 0,
           (sockaddr*)&serverAddr, sizeof(serverAddr));

    std::cout << "Packet sent!" << std::endl;
    close(sockfd);
}
