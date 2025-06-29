#include "Connection.hpp"       // Declaração da classe Connection
#include "UuidGenerator.hpp"    // Para gerar um UUID aleatório
#include <iostream>             // Para saída padrão (cout, cerr)
#include <cstdio>               // Para printf
#include <sys/socket.h>         // Funções de socket (sendto, recvfrom)
#include <netinet/in.h>         // Estrutura sockaddr_in
#include <unistd.h>             // Para close() e funções POSIX

// Realiza handshake com o servidor via protocolo SLOW
bool Connection::threeWayHandshake(int sockfd, sockaddr_in& server,
                                   std::array<uint8_t, 16>& out_sid,
                                   uint32_t& out_sttl, uint32_t& seqnum) {
    // 1) Monta o pacote CONNECT
    SLOWPacket connect;
    connect.sid = UuidGenerator::generate();    // Gera um SID aleatório para a sessão
    connect.sttl = 0;                           // TTL inicialmente zero (será definido pelo servidor)
    connect.flags = CONNECT;                    // Flag CONNECT (solicitação de conexão)
    connect.seqnum = 0;                         // Número de sequência inicial
    connect.acknum = 0;                         // Sem ACK neste pacote
    connect.window = 4096;                      // Janela de recepção do cliente
    connect.fid = 0;                            // Fragment ID
    connect.fo = 0;                             // Fragment offset

    // 2) Serializa o pacote CONNECT para vetor de bytes
    auto data = Serializer::serialize(connect);

    // 3) Mostra o conteúdo hexadecimal do CONNECT     *Usado apenas para testes
    std::cout << "📤 Pacote CONNECT serializado (" << data.size() << " bytes):\n";
    for (size_t i = 0; i < data.size(); ++i) {
        // imprima o índice (i) e o valor em hex deste byte
        printf(" [%02zu]=%02X", i, data[i]);
        if ((i + 1) % 8 == 0) std::cout << "\n";
    }
    std::cout << "\n\n";

    // 4) Envia pacote CONNECT via UDP
    socklen_t len = sizeof(server);
    ssize_t sent = sendto(sockfd, data.data(), data.size(),
                          0, (sockaddr*)&server, len);
    if (sent < 0) {
        perror("sendto failed"); // Erro ao enviar
        return false;
    }
    std::cout << "→ CONNECT enviado com " << sent << " bytes\n";

    // 5) Configura timeout de 5 segundos para recvfrom()
    struct timeval tv;
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // 6) Aguarda resposta (ACCEPT ou REJECT)
    std::vector<uint8_t> buffer(1500);  // Buffer para resposta
    std::cout << "Esperando resposta de ACCEPT...\n";
    ssize_t recvlen = recvfrom(sockfd, buffer.data(), buffer.size(),
                               0, nullptr, nullptr);
    if (recvlen < 0) {
        perror("recvfrom failed"); // Timeout ou erro de leitura
        return false;
    }

    // 7) Imprime os bytes brutos recebidos   *Usado apenas para testes
    if (recvlen > 0) {
        std::cout << "📥 Texto da resposta do servidor: ";
        for (size_t i = 32; i < (size_t)recvlen; ++i) {
            std::cout << static_cast<char>(buffer[i]);
        }
        std::cout << "\n";
    }

    // 8) Desserializa em um SLOWPacket
    SLOWPacket pkt = Serializer::deserialize(buffer);

    // 9) Verifica se vem flag ACCEPT
    if (!(pkt.flags & ACCEPT)) {
        std::cerr << "Connection not accepted\n";   // Falha na negociação
        return false;
    }

    // 10) Preenche variáveis de saída com informações da sessão
    out_sid    = pkt.sid;       // SID fornecido pelo servidor
    out_sttl   = pkt.sttl;      // TTL de validade da sessão
    seqnum     = pkt.seqnum + 1;// Próximo número de sequência a ser usado
    
    // Conexão estabelecida com sucesso
    return true;
}
