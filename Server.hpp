#pragma once

#include "Client.hpp"
#include "Config.hpp"
#include <map>
#include <netdb.h>
#include <string>
#include <sys/epoll.h>
#include <vector>

#define SOCKET_BUFFER_SIZE 4096
#define MAX_EPOLL_EVENTS 1024

#include <ctime>
#define DEBUG 1

class Server
{
  public:
    Server();
    ~Server();

    // bool setup(const ServerConfig &parsed_config);
    bool setup(const std::vector<ServerConfig> &parsed_configs);
    bool run();

  private:
    Server(const Server &other);
    Server &operator=(const Server &other);

    int create_socket();
    bool bind_socket(int server_fd, const ServerConfig &config);
    bool listen_socket(int server_fd, const ServerConfig &config);

    bool setup_epoll();
    bool add_epoll_fd(int fd, uint32_t events);
    bool modify_epoll_fd(int fd, uint32_t events);
    bool accept_client(int client_fd, size_t config_index);
    void close_client(int client_fd);
    bool handle_client_read(int client_fd);
    bool handle_client_write(int client_fd);
    void close_all_clients();

    // Tutte le configurazioni lette dai blocchi "server" del file .conf.
    std::vector<ServerConfig> configs;

    // Associa ogni listening socket all'indice della sua configurazione.
    // Esempio: listening_sockets[3] = 0 significa:
    // fd 3 ascolta usando configs[0].
    std::map<int, size_t> listening_sockets;

    // Associa ogni client alla configurazione del listening socket
    // sul quale è stata accettata la sua connessione.
    // Esempio: client_configs[7] = 0 significa:
    // il client fd 7 deve usare configs[0].
    std::map<int, size_t> client_configs;

    // Contiene gli oggetti Client, indicizzati tramite il loro fd.
    std::map<int, Client> clients;

    bool running;

    int epoll_fd;
};
