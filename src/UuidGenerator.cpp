#include "UuidGenerator.hpp"

// Não está sendo usado no momento, talvez seja útil no trabalho 2
// cria e retorna um UUID (identificador único universal) de 16 bytes
std::array<uint8_t, 16> UuidGenerator::generate() {
    std::array<uint8_t, 16> uuid;


    std::random_device rd; // Obtem semente aleatória do sistema
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 255); // gera inteiros aleatórios entre 0 e 255 (1 byte).

    for (auto& byte : uuid) byte = dist(gen);

    // Define os 4 bits mais significativos do byte 6 para indicar versão 8 (custom UUID)
    uuid[6] = (uuid[6] & 0x0F) | 0x80; 

    // Define os 2 bits mais significativos do byte 8 para indicar variante RFC4122
    uuid[8] = (uuid[8] & 0x3F) | 0x80; // variant RFC4122

    return uuid;
}