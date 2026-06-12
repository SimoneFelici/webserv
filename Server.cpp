#include "Server.hpp"
#include "Utils.hpp"

void Server::set_port(int parsed_port)
{
    port = parsed_port;
}

void Server::create_socket()
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        std::cerr << "Error creating socket\n";
        exit(1);
    }
}

void Server::bind_socket()
{
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    // TODO: From the parser check if INADDR_ANY or LOOPBACK
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if (!bind(fd, (struct sockaddr*)&server_address, sizeof(server_address))) {
        std::cerr << "Couldn't bind address: TODO: ERROR HANDLING / FREE RESOURCES / CHECK SUBJECT FOR ERRNO\n";
        exit(1);
    }
}

void Server::listen_socket()
{
    // TODO: CHECK IF MAX CONNECTION CAN BE SET FROM THE CONFIG
    if (!listen(fd, SOMAXCONN)) {
        std::cerr << "Couldn't listen for connections: TODO: ERROR HANDLING / FREE RESOURCES / CHECK SUBJECT FOR ERRNO\n";
    }
}

Server::Server()
{
    int opt = 1;
    create_socket();
    Utils::set_nonblocking(fd);
    if (!setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Couldn't set socket options :(\n";
    }
    bind_socket();
    listen_socket();
}

Server::~Server()
{
}
