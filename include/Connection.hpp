#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "Packet.hpp"
#include "Serializer.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

class Connection
{
public:
    static bool threeWayHandshake(int sockfd, sockaddr_in &server,
                                  std::array<uint8_t, 16> &out_sid,
                                  uint32_t &out_sttl, uint32_t &seqnum);

    static bool disconnect(int sockfd, sockaddr_in &server,
                           const std::array<uint8_t, 16> &sid,
                           uint32_t sttl, uint32_t &current_seqnum);
};

#endif