int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(7033);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    std::array<uint8_t, 16> sid;
    uint32_t sttl = 0;
    uint32_t seqnum = 0;

    if (Connection::threeWayHandshake(sockfd, server, sid, sttl, seqnum)) {
        std::cout << "Handshake OK. SID: ";
        for (auto b : sid) std::cout << std::hex << (int)b;
        std::cout << "\nTTL: " << sttl << " Seq: " << seqnum << std::endl;
    } else {
        std::cerr << "Handshake failed" << std::endl;
    }

    close(sockfd);
    return 0;
}
