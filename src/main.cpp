#include "Connection.hpp"    // Interface de conexão SLOW (handshake, disconnect)
#include "Fragmenter.hpp"    // Fragmentação de payloads grandes
#include "Serializer.hpp"    // Serialização dos pacotes SLOW
#include "UuidGenerator.hpp" // (Opcional) Geração de UUIDs

#include <iostream>    // Entrada/saída padrão
#include <vector>      // Uso de std::vector
#include <string>      // Manipulação de strings
#include <arpa/inet.h> // Manipulação de endereços IP
#include <unistd.h>    // Função close()
#include <netdb.h>     // Função getaddrinfo()
#include <cstring>     // memcpy e outros

int main(int argc, char *argv[])
{
    std::string server_ip = "slow.gmelodie.com"; // Hostname do servidor SLOW
    int port = 7033;                             // Porta padrão
    uint16_t window = 4096;                      // Tamanho da janela de recepção

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Cria socket UDP (AF_INET = IPv4)
    if (sockfd < 0)
    {
        perror("socket"); // Mostra erro se falhar
        return 1;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(port); // Converte porta para formato de rede

    // Resolver DNS do host
    addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_INET;      // Apenas IPv4
    hints.ai_socktype = SOCK_DGRAM; // Tipo UDP

    int err = getaddrinfo(server_ip.c_str(), nullptr, &hints, &res);
    if (err != 0 || res == nullptr)
    {
        std::cerr << "Erro ao resolver host " << server_ip << ": " << gai_strerror(err) << std::endl;
        return 1;
    }
    // Copia endereço IP resolvido para a estrutura sockaddr_in
    memcpy(&server.sin_addr, &((sockaddr_in *)res->ai_addr)->sin_addr, sizeof(in_addr));
    freeaddrinfo(res); // Libera memória alocada por getaddrinfo

    std::cout << "Iniciando conexão com " << server_ip << ":" << port << "...\n";

    std::array<uint8_t, 16> sid; // Buffer para armazenar o Session ID
    uint32_t confirmed_sttl = 0; // TTL da sessão retornada pelo servidor
    uint32_t seqnum = 0;         // Número de sequência inicial

    if (!Connection::threeWayHandshake(sockfd, server, sid, confirmed_sttl, seqnum))
    {
        std::cerr << "[ERRO] Handshake falhou ou não houve resposta.\n";
        close(sockfd);
        return 1;
    }

    std::cout << "[OK] Handshake aceito pelo servidor!\n"
              << "     SID (UUID): ";
    for (auto b : sid)
        std::cout << std::hex << (int)b;
    std::cout << "\n     TTL da sessão: " << std::dec << confirmed_sttl
              << "\n     Próximo seqnum:  " << seqnum << "\n";

    std::string mensagem = "Hello from SLOW peripheral!";           // Mensagem de exemplo
    std::vector<uint8_t> payload(mensagem.begin(), mensagem.end()); // Converte para vetor de bytes

    auto pacotes = Fragmenter::fragmentPayload(sid, confirmed_sttl, seqnum, window, payload);

    std::cout << "Enviando payload em " << pacotes.size() << " fragmento(s) ...\n";
    for (auto &pkt : pacotes)
    {
        auto buffer = Serializer::serialize(pkt); // Serializa pacote
        ssize_t sent = sendto(sockfd, buffer.data(), buffer.size(), 0, (sockaddr *)&server, sizeof(server));
        if (sent < 0)
        {
            perror("sendto"); // Falha no envio
            close(sockfd);
            return 1;
        }
        std::cout << "." << std::flush; // Progresso visual (ponto por pacote)
    }

    std::cout << "\nPayload enviado. Iniciando desconexão...\n";

    Connection::disconnect(sockfd, server, sid, confirmed_sttl, seqnum, seqnum);

    std::cout << "Programa encerrando.\n";
    close(sockfd); // Fecha o socket
    return 0;      // Sucesso
}
