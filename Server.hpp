#pragma once

#include "Client.hpp"
#include <map>
#include <netdb.h>
#include <string>
#include <sys/epoll.h>
#include <vector>

// TODO: REMOVE ONCE FINISHED TESTING
#include <ctime>
#define DEBUG 1

struct LocationConfig
{
    std::string path;
    std::string root;
    std::string index;
    bool autoindex;
    std::vector<std::string> allowed_methods;
    std::map<int, std::string> error_pages;
};

struct ServerConfig
{
    std::string address;
    std::string version;
    std::string port;
    std::string root;
    std::string index;
    int max_conn;
    std::map<int, std::string> error_pages;
    std::vector<std::string> allowed_methods;

    std::vector<LocationConfig> locations;
};

class Server
{
  public:
    Server();
    ~Server();

    bool setup(const char *conf_file);
    bool run();

  private:
    Server(const Server &other);
    Server &operator=(const Server &other);

    bool create_socket();
    bool bind_socket();
    bool listen_socket();

    bool parse_config(const char *conf_file);
    void set_port(std::string parsed_port);
    void set_address(std::string parsed_address);
    void set_max_conn(int parsed_max);

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
