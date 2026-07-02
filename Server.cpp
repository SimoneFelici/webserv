#include "Server.hpp"
#include "webserv.hpp"

Server::Server() : fd(-1), running(false), epoll_fd(-1)
{
}

// PARSING
void Server::set_port(std::string parsed_port) { this->config.port = parsed_port; }

void Server::set_address(std::string parsed_address)
{
    this->config.address = parsed_address;
}

void Server::set_max_conn(int parsed_max) { this->config.max_conn = parsed_max; }

// TODO: READ CONFIG FILE AND PARSE REAL VALUES
// TODO: BETTER OPEN ERROR LOGGING
// INFO: the listen field in the config file can be used onlt with the port
// without an address, in that case the default is "0.0.0.0" which listens to any address!
bool Server::parse_config(const char *conf_file)
{
    int conf_fd;

    conf_fd = open(conf_file, O_RDONLY);
    if (conf_fd < 0)
    {
        std::cerr << "Error: couldn't open config file\n";
        return (false);
    }
    set_port(PORT);
    // ADD LOG INFO...
    set_address(ADDRESS);
    // IF MAX_CONN <= 0 CHANGE IT TO 1, IF LARGER THAN SOMAXCON CHANGE IT TO SOMAXCON
    set_max_conn(MAX_CONN);
    // Da decidere cosa fare
    this->config.root = "./www";
    this->config.index = "index.html";
    LocationConfig root_location;

    root_location._location = "/";
    root_location.root = this->config.root;
    root_location.index = this->config.index;
    root_location.autoindex = false;
    root_location.allowed_methods.push_back("GET");

    this->config.locations.push_back(root_location);
    close(conf_fd);
    return (true);
}

// SOCKET
bool Server::create_socket()
{
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd == -1)
    {
        std::cerr << "Error: couldn't create socket: " << strerror(errno) << std::endl;
        return (false);
    }
    std::cout << "Success: Socket created, server fd: " << this->fd << "\n";
    return (true);
}

bool Server::bind_socket()
{
    struct addrinfo hints;
    struct addrinfo *res;
    const char *host = this->config.address.c_str();
    int err;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    res = NULL;
    err = getaddrinfo(host, this->config.port.c_str(), &hints, &res);
    if (err != 0)
    {
        std::cerr << "Error: getaddrinfo: " << gai_strerror(err) << std::endl;
        return (false);
    }
    if (bind(fd, res->ai_addr, res->ai_addrlen) == -1)
    {
        std::cerr << "Error: Couldn't bind address: " << strerror(errno) << std::endl;
        freeaddrinfo(res);
        return (false);
    }
    // TODO: MAYBE USE ADDRINFO ADDRES AND TRANSLATE IT?
    std::cout << "Success: Address binded, Address: " << this->config.address << ", port : " << this->config.port << "\n";
    freeaddrinfo(res);
    return (true);
}

bool Server::listen_socket()
{
    if (listen(fd, this->config.max_conn) == -1)
    {
        std::cerr << "Error: Couldn't listen for connections: " << strerror(errno) << std::endl;
        return (false);
    }
    std::cout << "Success: Socket listening, max connections : " << this->config.max_conn << "\n";
    return (true);
}

// CLEANUP SERVER
Server::~Server()
{
    // CLOSE EPOLL FD
    if (this->epoll_fd != -1)
        close(this->epoll_fd);
    // CLOSE SERVER FD
    if (this->fd != -1)
        close(this->fd);
}

// EPOLL
bool Server::setup_epoll()
{
    this->epoll_fd = epoll_create(1);
    if (this->epoll_fd == -1)
    {
        std::cerr << "Error: epoll_create failed: " << strerror(errno) << std::endl;
        return false;
    }

    if (!add_epoll_fd(this->fd, EPOLLIN))
        return false;

    return true;
}

bool Server::add_epoll_fd(int fd, uint32_t events)
{
    epoll_event event;

    event.events = events;
    event.data.fd = fd;
    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &event) == -1)
    {
        std::cerr << "Error: epoll_ctl ADD failed: " << strerror(errno) << std::endl;
        return (false);
    }
    return (true);
}

bool Server::modify_epoll_fd(int fd, uint32_t events)
{
    epoll_event event;

    event.events = events;
    event.data.fd = fd;
    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, fd, &event) == -1)
    {
        std::cerr << "Error: epoll_ctl modify failed: " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

// CLIENT
bool Server::accept_client(int client_fd)
{
    if (!set_nonblocking(client_fd))
        return (false);
    if (!add_epoll_fd(client_fd, EPOLLIN))
        return (false);
    this->clients[client_fd] = Client(client_fd);
    return (true);
}

void Server::close_client(int client_fd)
{
    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    close(client_fd);
    this->clients.erase(client_fd);
}

void Server::close_all_clients()
{
    while (!this->clients.empty())
    {
        close_client(this->clients.begin()->first);
    }
}

bool Server::handle_client_read(int client_fd)
{
    std::map<int, Client>::iterator it = this->clients.find(client_fd);
    if (it == this->clients.end())
        return false;

    Client &client = it->second; // client è un riferimento al Client dentro la map, quindi quando fai append modifichi davvero quel client.
    char temp[4096];

    ssize_t bytes_read = recv(client_fd, temp, sizeof(temp), 0);

    if (bytes_read <= 0)
    {
        std::cout << "recv failed and/or Client disconnected: " << client_fd << std::endl;
        return false;
    }

    if (!client.has_full_headers(temp, bytes_read))
        return true;
    if (!client.parse_request())
        return false;
    if (client.req_done())
    {
        if (DEBUG)
            client.print_request();

        if (!client.prepare_response(this->config))
            return false;

        if (!modify_epoll_fd(client_fd, EPOLLOUT))
            return false;
    }

    return true;
}

bool Server::handle_client_write(int client_fd)
{
    std::map<int, Client>::iterator it = this->clients.find(client_fd);
    if (it == this->clients.end())
        return false;

    Client &client = it->second;

    const std::string &response = client.get_response();

    std::size_t bytes_sent = client.get_bytes_sent();
    if (bytes_sent > response.size())
        return false;
    std::size_t bytes_left = response.size() - bytes_sent;

    /*
    response.c_str() + bytes_sent : vuol dire: parti dal punto in cui eri rimasta.
    response.size() - bytes_sent : manda solo quello che manca */
    ssize_t sent = send(client_fd, response.c_str() + bytes_sent, bytes_left, 0);
    if (sent <= 0)
        return false;

    client.add_bytes_sent(sent); // aggiorno quanti byte sono stati davvero mandati.

    if (client.clear_response()) // torna true se la risposta è stata mandata tutta
        close_client(client_fd);
    return true;
}

bool Server::run()
{
    const int max_events = 1024;
    int client_fd;

    this->running = true;
    epoll_event events[max_events]; // array dove epoll_wait() scriverà gli eventi pronti.

    // DEBUG: remove after testing
    // time_t start = time(NULL);
    while (this->running)
    {
        // DEBUG: stoppo il server dopo 5 secondi per non doverlo killare ogni volta.
        // if ((DEBUG) && (time(NULL) - start >= 5))
        //     this->running = false;

        int ready = epoll_wait(this->epoll_fd, events, max_events, -1); // ready è il numero di eventi pronti.
        if (ready == -1)
        {
            if (errno == EINTR)
                continue;
            std::cerr << "Error: epoll_wait failed: " << strerror(errno) << std::endl;
            close_all_clients();
            return false;
        }
        for (int i = 0; i < ready; ++i)
        {
            int current_fd = events[i].data.fd;  // current_fd è il fd su cui è successo qualcosa.
            uint32_t revents = events[i].events; // revents contiene cosa è successo:
            if (current_fd == this->fd && (revents & (EPOLLERR | EPOLLHUP)))
            {
                std::cerr << "Error: server socket epoll event failed" << std::endl;
                close_all_clients();
                return false;
            }
            if (current_fd != this->fd && (revents & (EPOLLERR | EPOLLHUP)))
            {
                close_client(current_fd);
                continue;
            }
            if (revents & EPOLLIN)
            {
                if (current_fd == this->fd) // se è l fd corrente quello del server sono nuove connessioni
                {
                    client_fd = accept(this->fd, NULL, NULL);
                    if (client_fd == -1)
                    {
                        std::cerr << "Error: accept failed: " << strerror(errno) << std::endl;
                        continue;
                    }
                    if (!accept_client(client_fd))
                    {
                        close(client_fd);
                        continue;
                    }
                    std::cout << "Client connected, fd: " << client_fd << "\n";
                }
                else
                {
                    if (!handle_client_read(current_fd)) // DA IMPLEMENTARE
                    {
                        close_client(current_fd);
                        continue;
                    }
                }
            }
            // DA IMPLEMENTARE
            if (revents & EPOLLOUT)
            {
                if (current_fd != this->fd)
                {
                    if (!handle_client_write(current_fd))
                        close_client(current_fd);
                    continue;
                }
            }
        }
    }
    close_all_clients();
    return true;
}

bool Server::setup(const char *conf_file)
{
    int opt;

    if (!parse_config(conf_file))
        return false;
    if (!create_socket())
        return false;
    if (!set_nonblocking(fd))
        return false;

    opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Error: Couldn't set reuse address socket option: " << strerror(errno) << std::endl;
        return false;
    }
    std::cout << "Success: Reuse address socket option enabled" << '\n';

    if (!bind_socket())
        return false;
    if (!listen_socket())
        return false;
    if (!setup_epoll())
        return false;

    return true;
}
