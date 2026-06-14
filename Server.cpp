#include "webserv.hpp"

// PARSING
void Server::set_port(std::string parsed_port)
{
    this->port = parsed_port;
}

void Server::set_address(std::string parsed_address)
{
    this->address = parsed_address;
}

void Server::set_max_conn(int parsed_max)
{
    this->max_conn = parsed_max;
}

// TODO: READ CONFIG FILE AND PARSE REAL VALUES
// TODO: BETTER OPEN ERROR LOGGING
// INFO: the listen field in the config file can be used onlt with the port without an address, in that case the default is "0.0.0.0" which listens to any address!
bool Server::parse_config(const char* conf_file)
{
    int conf_fd = open(conf_file, O_RDONLY);

    if (conf_fd < 0) {
        std::cerr << "Error: couldn't open config file\n";
        return false;
    }

    set_port(PORT);
    // ADD LOG INFO...
    set_address(ADDRESS);
    // IF MAX_CONN <= 0 CHANGE IT TO 1, IF LARGER THAN SOMAXCON CHANGE IT TO SOMAXCON
    set_max_conn(MAX_CONN);

    close(conf_fd);

    return true;
}

// SOCKET
bool Server::create_socket()
{
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd == -1) {
        std::cerr << "Error: couldn't create socket: " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "Success: Socket created, server fd: " << this->fd << "\n";
    return true;
}

bool Server::bind_socket()
{
    // TODO: UPDATE NOTION WITH GETADDRINFO INFO
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = NULL;
    const char* host = this->address.c_str();

    int err = getaddrinfo(host, this->port.c_str(), &hints, &res);
    if (err != 0) {
        std::cerr << "Error: getaddrinfo: " << gai_strerror(err) << std::endl;
        return false;
    }

    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1) {
        std::cerr << "Error: Couldn't bind address: " << strerror(errno) << std::endl;
        freeaddrinfo(res);
        return false;
    }

    // TODO: MAYBE USE ADDRINFO ADDRES AND TRANSLATE IT?
    std::cout << "Success: Address binded, Address: " << this->address << ", port: " << this->port << "\n";
    freeaddrinfo(res);
    return true;
}

bool Server::listen_socket()
{
    if (listen(fd, this->max_conn) == -1) {
        std::cerr << "Error: Couldn't listen for connections: " << strerror(errno) << std::endl;
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
    , max_conn(0)
{
}

bool Server::start(const char* conf_file)
{
    if (!parse_config(conf_file))
        return false;

    if (!create_socket())
        return false;

    if (!set_nonblocking(fd))
        return false;

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Error: Couldn't set reuse address socket option: " << strerror(errno) << std::endl;
        return false;
    }
    std::cout << "Success: Reuse address socket option enabled\n";

    if (!bind_socket())
        return false;

    if (!listen_socket())
        return false;

    return true;
}
