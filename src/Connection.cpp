#include "Connection.hpp"    // Definição da classe Connection e métodos
#include "UuidGenerator.hpp" // Geração de UUIDs
#include <iostream>          // Saída padrão (cout, cerr)
#include <cstdio>            // Funções de C como printf
#include <sys/socket.h>      // API de sockets
#include <netinet/in.h>      // Estruturas de endereço (sockaddr_in)
#include <unistd.h>          // Funções POSIX como close()
#include <cstring>           // Manipulação de memória (memset, memcpy etc.)

// Função responsável por realizar o three-way handshake com o servidor
bool Connection::threeWayHandshake(int sockfd, sockaddr_in &server,
                                   std::array<uint8_t, 16> &out_sid,
                                   uint32_t &out_sttl, uint32_t &seqnum)
{
    // 1) Monta o pacote CONNECT usando Serializer
    SLOWPacket connect;
    connect.sid.fill(0);     // ID da sessão vazio inicialmente
    connect.sttl = 0;        // TTL do servidor
    connect.flags = CONNECT; // 0x04 Flag CONNECT
    connect.seqnum = 0;      // Número de sequência inicial
    connect.acknum = 0;      // Número de ACK inicial
    connect.window = 8220;   // Tamanho da janela de recepção
    connect.fid = 0;         // File ID, caso haja envio de arquivos
    connect.fo = 0;          // File offset

    // Serializa o pacote CONNECT para vetor de bytes
    auto data = Serializer::serialize(connect);
    // Exibe bytes serializados para debug
    std::cout << "📤 Pacote CONNECT serializado (" << data.size() << " bytes):\n";
    for (size_t i = 0; i < data.size(); ++i)
    {
        printf(" [%02zu]=%02X", i, data[i]);
        if ((i + 1) % 8 == 0)
            std::cout << "\n";
    }

    // 2) Envia CONNECT via UDP
    socklen_t len = sizeof(server);
    ssize_t sent = sendto(sockfd, data.data(), data.size(), 0, (sockaddr *)&server, len);
    if (sent < 0)
    {
        perror("sendto failed"); // Erro ao enviar
        return false;
    }
    // std::cout << "→ CONNECT enviado com " << sent << " bytes\n";    Apenas para debug

    // 3) Timeout de 5s para receber SETUP
    struct timeval tv{5, 0};
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Cria buffer para armazenar resposta do servidor
    std::vector<uint8_t> buffer(1500); // Tamanho máximo típico de um pacote UDP
    std::cout << "Esperando resposta de ACCEPT...\n";

    // 4) Aguarda pacote SETUP do servidor
    ssize_t recvlen = recvfrom(sockfd, buffer.data(), buffer.size(), 0, nullptr, nullptr);
    if (recvlen < 0)
    {
        perror("recvfrom failed");
        return false;
    }

    // 5) Imprime resposta textual do servidor
    std::cout << "📥 Texto da resposta do servidor: ";
    for (size_t i = 32; i < (size_t)recvlen; ++i)
        std::cout << static_cast<char>(buffer[i]);
    std::cout << "\n";

    // 6) Desserializa resposta (SETUP)
    SLOWPacket pkt = Serializer::deserialize(buffer);

    // Verifica se a flag ACK está presente no pacote SETUP
    if (!(pkt.flags & ACK))
    {
        std::cerr << "Connection not accepted\n";
        return false;
    }

    // 7) Preenche as variáveis de saída com SETUP
    out_sid = pkt.sid;
    out_sttl = pkt.sttl;
    seqnum = pkt.seqnum + 1;

    // 8) ACK vazio de confirmação do setup
    SLOWPacket ack;
    ack.sid = out_sid;
    ack.sttl = out_sttl;
    ack.flags = ACK;         // Apenas ACK
    ack.seqnum = seqnum;     // Número de sequência do cliente
    ack.acknum = pkt.seqnum; // Confirmando pacote do servidor
    ack.window = 4096;       // Tamanho da janela
    ack.fid = 0;
    ack.fo = 0;
    ack.data.clear(); //  IMPORTANTE: sem dados

    // Serializa e envia o pacote ACK
    auto ackdata = Serializer::serialize(ack);
    sendto(sockfd, ackdata.data(), ackdata.size(), 0, (sockaddr *)&server, len);
    std::cout << "→ ACK enviado sem dados. Aguardando confirmação...\n";

    // 9) Espera o ACK do servidor confirmando
    recvlen = recvfrom(sockfd, buffer.data(), buffer.size(), 0, nullptr, nullptr);
    if (recvlen < 0)
    {
        perror("recvfrom failed (ACK confirm)");
        return false;
    }

    std::cout << "📥 ACK final recebido. Conexão estabelecida com sucesso.\n";
    seqnum++; // Incrementa seqnum para envio de dados reais
    return true;
}
// Função que realiza desconexão segura da sessão
void Connection::disconnect(int sockfd, const sockaddr_in &server,
                            const std::array<uint8_t, 16> &sid,
                            uint32_t sttl, uint32_t seqnum, uint32_t acknum)
{
    SLOWPacket disconnect_pkt;
    disconnect_pkt.sid = sid;
    disconnect_pkt.sttl = sttl;
    disconnect_pkt.flags = ACK | CONNECT | REVIVE;
    disconnect_pkt.seqnum = seqnum;
    disconnect_pkt.acknum = acknum;
    disconnect_pkt.window = 0;
    disconnect_pkt.fid = 0;
    disconnect_pkt.fo = 0;
    disconnect_pkt.data.clear();

    // Serializa o pacote
    auto data = Serializer::serialize(disconnect_pkt);

    std::cout << "Enviando pacote DISCONNECT..." << std::endl;

    // Envia o pacote para o servidor
    ssize_t sent = sendto(sockfd, data.data(), data.size(), 0, (const sockaddr *)&server, sizeof(server));
    if (sent < 0)
    {
        perror("sendto (disconnect) failed");
    }
    else
    {
        std::cout << "-> Pacote DISCONNECT enviado com " << sent << " bytes." << std::endl;
    }
}