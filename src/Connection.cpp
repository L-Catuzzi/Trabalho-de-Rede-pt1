#include "Connection.hpp"
#include "UuidGenerator.hpp"
#include <iostream>

bool Connection::threeWayHandshake(int sockfd, sockaddr_in& server,
                                   std::array<uint8_t, 16>& out_sid,
                                   uint32_t& out_sttl, uint32_t& seqnum) {
    SLOWPacket connect;
    connect.sttl = 0;
    connect.flags = CONNECT;
    connect.seqnum = 0;
    connect.window = 4096;
    connect.sid = UuidGenerator::generate();
    
    auto data = Serializer::serialize(connect);
    socklen_t len = sizeof(server);
    ssize_t sent = sendto(sockfd, data.data(), data.size(), 0, (sockaddr*)&server, len);
    if (sent < 0) {
    perror("sendto failed");
    return false;
    }
    std::cout << "Pacote CONNECT enviado com " << sent << " bytes\n";// Teste

    std::vector<uint8_t> buffer(1500);

    struct timeval tv;
    tv.tv_sec = 5;  // 5 segundos
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    std::cout << "Esperando resposta de ACCEPT...\n";// Teste

    ssize_t received = recvfrom(sockfd, buffer.data(), buffer.size(), 0, nullptr, nullptr);
    if (received < 0) {
        perror("recvfrom failed");
        return false;
    }


    auto reply = Serializer::deserialize(buffer);
    if (!(reply.flags & ACCEPT)) {
        std::cerr << "Connection not accepted" << std::endl;
        return false;
    }

    out_sid = reply.sid;
    out_sttl = reply.sttl;
    seqnum = reply.seqnum + 1;
    return true;
}
