#pragma once

#include "Utils.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8080
#define ADDRESS INADDR_ANY
#define MAX_CONN SOMAXCONN

class Server {
public:
    Server();
    ~Server();

    bool start();

private:
    Server(const Server& other);
    Server& operator=(const Server& other);

    bool create_socket();
    bool bind_socket();
    bool listen_socket();

    void parser();
    void set_port(int parsed_port);
    void set_address(uint32_t parsed_address);
    void set_max_conn(int parsed_max);

    int fd;
    int port;
    uint32_t address;
    int max_conn;
};
