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
bool Server::parse_config(const char *conf_file)
{
    int conf_fd = open(conf_file, O_RDONLY);

    if (conf_fd < 0)
    {
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
    if (this->fd == -1)
    {
        std::cerr << "Error: couldn't create socket: " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "Success: Socket created, server fd: " << this->fd << "\n";
    return true;
}

bool Server::bind_socket()
{
    struct addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res = NULL;
    const char *host = this->address.c_str();

    int err = getaddrinfo(host, this->port.c_str(), &hints, &res);
    if (err != 0)
    {
        std::cerr << "Error: getaddrinfo: " << gai_strerror(err) << std::endl;
        return false;
    }

    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1)
    {
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
    if (listen(fd, this->max_conn) == -1)
    {
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
    : fd(-1), port(), address(), max_conn(0), running(false)
{
}

bool Server::accept_client(int client_fd)
{
    if (!set_nonblocking(client_fd))
        return false;

    pollfd client_poll;
    client_poll.fd = client_fd;
    client_poll.events = POLLIN;
    client_poll.revents = 0;

    this->poll_fds.push_back(client_poll);
    this->clients[client_fd] = Client(client_fd);

    return true;
}

void Server::handle_client(int client_fd)
{
    char buffer[4096];

    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

    if (bytes_read > 0)
    {
        // ho letto dati HTTP dal client
        // per ora posso stamparli o salvarli nel Client
    }
    else if (bytes_read == 0)
    {
        // il client ha chiuso
        // devo chiudere fd e rimuovere client
    }
    else
    {
        // errore
        // se non è temporaneo, chiudo e rimuovo client
    }
}

bool Server::run()
{
    pollfd server_poll;
    server_poll.fd = this->fd;
    server_poll.events = POLLIN; 
    server_poll.revents = 0;     // eventi restituiti da poll()
    
    this->poll_fds.push_back(server_poll);
    
    this->running = true;
    while (this->running)
    {
        // poll()
        int ready = poll(&this->poll_fds[0], this->poll_fds.size(), -1); // punto di partenza dell'array + contiene il numero di fd pronti.
        if (ready == -1)
        {
            // if (errno == EINTR) da gestire??
            //     continue;
            std::cerr << "Error: poll failed: " << strerror(errno) << std::endl;
            return false;
        }

        // accept
        size_t count = this->poll_fds.size();
        for (size_t i = 0; i < count; ++i)
        {
            short revents = this->poll_fds[i].revents;
            
            if (revents == 0)
                continue;

            if (this->poll_fds[i].fd == this->fd && (revents & (POLLERR | POLLHUP | POLLNVAL)))
            {
                std::cerr << "Error: server socket poll event failed\n";
                return false;
            }
            
            if (revents & POLLIN)
            {
                if (this->poll_fds[i].fd == this->fd)
                {
                    // server fd pronto: qui faremo accept()
                    int client_fd = accept(this->fd, NULL, NULL);

                    if (client_fd == -1)
                    {
                        std::cerr << "Error: accept failed\n";
                        continue;
                    }
                    if (!accept_client(client_fd))
                    {
                        close(client_fd);
                        continue;
                    }
                }
                else
                {
                    handle_client(this->poll_fds[i].fd);
                }
            }
        }
    }
    return true;
}

bool Server::start(const char *conf_file)
{
    if (!parse_config(conf_file))
        return false;

    if (!create_socket())
        return false;

    if (!set_nonblocking(fd))
        return false;

    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        std::cerr << "Error: Couldn't set reuse address socket option: " << strerror(errno) << std::endl;
        return false;
    }
    std::cout << "Success: Reuse address socket option enabled\n";

    if (!bind_socket())
        return false;

    if (!listen_socket())
        return false;

    // return run_socket() ?
    return true;
}
