#pragma once

#include "Client.hpp"
#include <map>
#include <netdb.h>
#include <string>
#include <sys/epoll.h>
#include <vector>

class Server {
  public:
    Server();
    ~Server();

    bool start(const char *conf_file);

  private:
    Server(const Server &other);
    Server &operator=(const Server &other);

    bool create_socket();
    bool bind_socket();
    bool listen_socket();
    bool run();

    bool parse_config(const char *conf_file);
    void set_port(std::string parsed_port);
    void set_address(std::string parsed_address);
    void set_max_conn(int parsed_max);

    bool add_epoll_fd(int fd, uint32_t events);
    bool accept_client(int client_fd);
    void close_client(int client_fd);
    bool handle_client_read(int client_fd);

    int fd;
    std::string port;
    std::string address;
    int max_conn;
    bool running;

    int epoll_fd;
    std::map<int, Client> clients;
};
