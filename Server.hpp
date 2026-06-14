#pragma once

#include <netdb.h>
#include <string>

class Server {
public:
    Server();
    ~Server();

    bool start(const char* conf_file);

private:
    Server(const Server& other);
    Server& operator=(const Server& other);

    bool create_socket();
    bool bind_socket();
    bool listen_socket();

    bool parse_config(const char* conf_file);
    void set_port(std::string parsed_port);
    void set_address(std::string parsed_address);
    void set_max_conn(int parsed_max);

    int fd;
    std::string port;
    std::string address;
    int max_conn;
};
