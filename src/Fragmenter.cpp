#include "Fragmenter.hpp"       // Declaração da classe Fragmenter
#include <cstdlib>               // Para uso da função rand()

//Utilizado para dividir a mensagem em vários pacotes quando esta escede 1456 bytes

std::vector<SLOWPacket> Fragmenter::fragmentPayload(
    const std::array<uint8_t, 16>& sid,     // ID da sessão
    uint32_t sttl,                          // TTL da sessão
    uint32_t& seqStart,                     // Número de sequência inicial (modificado por referência)
    uint16_t window,                        // Janela de recepção
    const std::vector<uint8_t>& payload) {  // Dados brutos a serem fragmentados

    const size_t max_data = 1440;        // Tamanho máximo de dados por pacote (evita MTU overflow)
    // Gera um identificador de fragmento aleatório (entre 0 e 254)
    uint8_t fid = static_cast<uint8_t>(rand() % 255);


    std::vector<SLOWPacket> fragments;      // Vetor que armazenará os pacotes fragmentados
    // offset = índice no payload; fo = file offset (ordem do fragmento)
    for (size_t offset = 0, fo = 0; offset < payload.size(); ++fo) {
         // Calcula o tamanho do fragmento atual (sem ultrapassar o final do payload)
        size_t chunk = std::min(max_data, payload.size() - offset);

        SLOWPacket pkt;             // Cria um novo pacote fragmentado
        pkt.sid = sid;              // Copia SID da sessão
        pkt.sttl = sttl;            // TTL da sessão
        pkt.flags = SLOWFlags::ACK | SLOWFlags::MB;// Marca como fragmento com ACK e MB (multi-bloco)
        pkt.seqnum = seqStart++;    // Atribui seqnum único e incrementa para o próximo
        pkt.acknum = 0;             // Sem ACK neste caso
        pkt.window = window;        // Define janela de recepção
        pkt.fid = fid;              // Identificador do fragmento
        pkt.fo = static_cast<uint8_t>(fo);  // Ordem do fragmento no conjunto
        
        // Copia os dados do fragmento para o campo data do pacote
        pkt.data.insert(pkt.data.begin(),
                        payload.begin() + offset,
                        payload.begin() + offset + chunk);

        offset += chunk;        // Avança o offset no payload original
        
        // Se este for o último fragmento, remove flag MB (não é mais multibloco)
        if(offset >= payload.size()) pkt.flags &= ~SLOWFlags::MB;
        // Adiciona o pacote ao vetor de fragmentos
        fragments.push_back(pkt);
    }
    // Retorna o vetor com todos os pacotes prontos para envio
    return fragments;
}