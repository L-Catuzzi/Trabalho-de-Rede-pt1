#ifndef UUIDGENERATOR_HPP
#define UUIDGENERATOR_HPP

#include <array>
#include <random>
#include <cstdint>

class UuidGenerator {
public:
    static std::array<uint8_t, 16> generate();
};

#endif
