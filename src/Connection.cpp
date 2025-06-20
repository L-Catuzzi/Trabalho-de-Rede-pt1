#include "Connection.hpp"
#include "UuidGenerator.hpp"
#include <iostream>
#include <cstdio>      
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

bool Connection::threeWayHandshake(int sockfd, sockaddr_in& server,
                                   std::array<uint8_t, 16>& out_sid,
                                   uint32_t& out_sttl, uint32_t& seqnum) {
    // 1) Monta o pacote CONNECT usando Serializer
    SLOWPacket connect;
    connect.sid.fill(0);
    connect.sttl = 0;             
    connect.flags = CONNECT;      // 0x04
    connect.seqnum = 0;
    connect.acknum = 0;
    connect.window = 8220;
    connect.fid = 0;
    connect.fo = 0;           

    auto data = Serializer::serialize(connect);

    std::cout << "📤 Pacote CONNECT serializado (" << data.size() << " bytes):\n";
    for (size_t i = 0; i < data.size(); ++i) {
        printf(" [%02zu]=%02X", i, data[i]);
        if ((i + 1) % 8 == 0) std::cout << "\n";
    }

    // 2) Envia CONNECT via UDP
    socklen_t len = sizeof(server);
    ssize_t sent = sendto(sockfd, data.data(), data.size(), 0, (sockaddr*)&server, len);
    if (sent < 0) {
        perror("sendto failed");
        return false;
    }
    // std::cout << "→ CONNECT enviado com " << sent << " bytes\n";    Apenas para debug

    // 3) Timeout de 5s para receber SETUP
    struct timeval tv{5, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    std::vector<uint8_t> buffer(1500);
    std::cout << "Esperando resposta de ACCEPT...\n";
    ssize_t recvlen = recvfrom(sockfd, buffer.data(), buffer.size(), 0, nullptr, nullptr);
    if (recvlen < 0) {
        perror("recvfrom failed");
        return false;
    }

    // 4) Imprime resposta textual do servidor
    std::cout << "📥 Texto da resposta do servidor: ";
    for (size_t i = 32; i < (size_t)recvlen; ++i) std::cout << static_cast<char>(buffer[i]);
    std::cout << "\n";

    // 5) Desserializa resposta (SETUP)
    SLOWPacket pkt = Serializer::deserialize(buffer);

    if (!(pkt.flags & ACK)) {
        std::cerr << "Connection not accepted\n";
        return false;
    }

    // 6) Preenche as variáveis de saída com SETUP
    out_sid = pkt.sid;
    out_sttl = pkt.sttl;
    seqnum = pkt.seqnum + 1;

    // 7) ACK vazio de confirmação do setup
    SLOWPacket ack;
    ack.sid = out_sid;
    ack.sttl = out_sttl;
    ack.flags = ACK;
    ack.seqnum = seqnum;
    ack.acknum = pkt.seqnum;
    ack.window = 4096;
    ack.fid = 0;
    ack.fo = 0;
    ack.data.clear();  // ⚠️ IMPORTANTE: sem dados

    auto ackdata = Serializer::serialize(ack);
    sendto(sockfd, ackdata.data(), ackdata.size(), 0, (sockaddr*)&server, len);
    std::cout << "→ ACK enviado sem dados. Aguardando confirmação...\n";

    // 8) Espera o ACK do servidor confirmando
    recvlen = recvfrom(sockfd, buffer.data(), buffer.size(), 0, nullptr, nullptr);
    if (recvlen < 0) {
        perror("recvfrom failed (ACK confirm)");
        return false;
    }

    std::cout << "📥 ACK final recebido. Conexão estabelecida com sucesso.\n";
    seqnum++;  // pronto para enviar dados
    return true;
}

