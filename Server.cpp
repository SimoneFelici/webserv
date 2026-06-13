#include "webserv.hpp"

// PARSING
void Server::set_port(int parsed_port)
{
    this->port = parsed_port;
}

void Server::set_address(uint32_t parsed_address)
{
    this->address = parsed_address;
}

void Server::set_max_conn(int parsed_max)
{
    this->max_conn = parsed_max;
}

void Server::parser()
{
    // READ CONFIG FILE...
    set_port(PORT);
    // ADD LOG INFO...
    set_address(ADDRESS);
    set_max_conn(MAX_CONN);
}

// SOCKET
bool Server::create_socket()
{
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd == -1) {
        std::cerr << "Error: couldn't create socket\n";
        return false;
    }

    std::cout << "Success: Socket created, server fd: " << this->fd << "\n";
    return true;
}

bool Server::bind_socket()
{
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(this->port);
    server_address.sin_addr.s_addr = htonl(this->address);

    if (bind(fd, (struct sockaddr*)&server_address, sizeof(server_address))) {
        std::cerr << "Error: Couldn't bind address\n";
        return false;
    }

    std::cout << "Success: Address binded, Address: " << this->address << ", port: " << this->port << "\n";
    return true;
}

bool Server::listen_socket()
{
    if (listen(fd, this->max_conn)) {
        std::cerr << "Error: Couldn't listen for connections\n";
        return false;
    }

    std::cout << "Success: Socket listening, max connections: " << this->max_conn << "\n";
    return true;
}

// CLEANUP SERVER
Server::~Server()
{
    if (this->fd >= 0)
        close(this->fd);
}

// SERVER
Server::Server()
    : fd(-1)
    , port(0)
    , address(0)
    , max_conn(0)
{
}

bool Server::start()
{
    parser();

    if (!create_socket())
        return false;

    set_nonblocking(fd);

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Error: Couldn't set reuse address socket option :(\n";
        return false;
    }
    std::cout << "Success: Reuse address socket option enabled\n";

    if (!bind_socket())
        return false;

    if (!listen_socket())
        return false;

    return true;
}
