#include "UuidGenerator.hpp"

std::array<uint8_t, 16> UuidGenerator::generate() {
    std::array<uint8_t, 16> uuid;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 255);

    for (auto& byte : uuid) byte = dist(gen);

    uuid[6] = (uuid[6] & 0x0F) | 0x80; // version 8 (custom)
    uuid[8] = (uuid[8] & 0x3F) | 0x80; // variant RFC4122

    return uuid;
}