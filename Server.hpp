#pragma once

#include "Utils.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080

class Server {
public:
    Server();
    ~Server();
    Server(const Server& other) = delete;
    Server& operator=(const Server& other) = delete;

    void set_port(int parsed_port);

private:
    void create_socket();
    void bind_socket();
    void listen_socket();
    int fd;
    int port;
};
