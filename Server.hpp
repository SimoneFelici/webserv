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

    bool setup(const ServerConfig &parsed_config);
    bool run();

  private:
    Server(const Server &other);
    Server &operator=(const Server &other);

    bool create_socket();
    bool bind_socket();
    bool listen_socket();

    bool setup_epoll();
    bool add_epoll_fd(int fd, uint32_t events);
    bool modify_epoll_fd(int fd, uint32_t events);
    bool accept_client(int client_fd);
    void close_client(int client_fd);
    bool handle_client_read(int client_fd);
    bool handle_client_write(int client_fd);
    void close_all_clients();

    int fd;

    ServerConfig config;

    bool running;

    int epoll_fd;
    std::map<int, Client> clients;
};
