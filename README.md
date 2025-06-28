Primeira parte do trabalho final de redes de computadores

## Integrantes
Luiz Fellipe Catuzzi - 11871198
Eduarda Tuboy Nardin - 13732495
Rafael Brazolin Alves Mansur - 14604020

## Objetivo
Este projeto implementa o protocolo SLOW (Simple Layered Over UDP Wrapper) usando C++17. A seguir estão as bibliotecas utilizadas no projeto, suas finalidades e onde são aplicadas:

## Dependências
### Bibliotecas POSIX (requerem Linux/WSL2/macOS/MSYS2)

| Biblioteca         | Para que serve                                      | Onde é usada                  |
|--------------------|------------------------------------------------------|----------------------------------------|
| `<sys/socket.h>`   | Criação e controle de sockets UDP                   | Envio/recebimento de pacotes           |
| `<netinet/in.h>`   | Estruturas para endereços IP e portas (`sockaddr_in`) | Definição de endereço do servidor      |
| `<arpa/inet.h>`    | Conversão entre texto/IP binário (`inet_pton`)      | IP string → binário                    |
| `<unistd.h>`       | Funções POSIX como `close()`                        | Fechamento do socket                   |

## Observações
- Este projeto **não roda nativamente em Windows com MinGW**, pois MinGW não inclui as bibliotecas POSIX. Use **WSL2**, ou um sistema Linux.
- Nenhuma biblioteca externa precisa ser instalada manualmente (como via vcpkg ou conan).

## Como compilar
```bash
make          # Compila o binário principal em bin/slow-peripheral
```

## Como executar
```bash
./bin/slow-peripheral --server 127.0.0.1 --port 7033 --buffer-size 4096 --session-ttl 30000
```
## Resultados finais
