#include "Config.hpp"
#include "Server.hpp"
#include "webserv.hpp"

Server::Server() : running(false), epoll_fd(-1)
{
}
Server::~Server()
{
    close_all_clients();

    for (std::map<int, size_t>::iterator it = this->listening_sockets.begin(); it != this->listening_sockets.end(); ++it)
        close(it->first);

    this->listening_sockets.clear();

    if (this->epoll_fd != -1)
    {
        close(this->epoll_fd);
        this->epoll_fd = -1;
    }
}

// SOCKET
// Ogni blocco "server" della configurazione deve avere
// il proprio socket sul quale ascoltare.
int Server::create_socket()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        std::cerr << "Error: couldn't create socket: " << strerror(errno) << std::endl;
        return -1;
    }
    std::cout << "Success: socket created, server fd: " << server_fd << std::endl;

    // Restituiamo il nuovo fd invece di salvarlo in this->fd,
    // perché ora Server gestisce più listening socket.
    return server_fd;
}

// Collega questo specifico socket all'indirizzo e alla porta
// contenuti nella configurazione che gli appartiene.
bool Server::bind_socket(int server_fd, const ServerConfig &config)
{
    struct addrinfo hints;
    struct addrinfo *res;
    int err;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    res = NULL;
    // Ricava la struttura di rete usando address e port
    // del blocco server corrente.
    err = getaddrinfo(config.address.c_str(), config.port.c_str(), &hints, &res);
    if (err != 0)
    {
        std::cerr << "Error: getaddrinfo: " << gai_strerror(err) << std::endl;
        return false;
    }
    // Esegue il bind sul socket corrente.
    // Ogni listening socket viene quindi associato
    // alla propria coppia address:port.
    if (bind(server_fd, res->ai_addr, res->ai_addrlen) == -1)
    {
        std::cerr << "Error: couldn't bind " << config.address << ":" << config.port << ": " << strerror(errno) << std::endl;
        freeaddrinfo(res);
        return false;
    }

    freeaddrinfo(res);

    std::cout << "Success: address bound, address: " << config.address << ", port: " << config.port << std::endl;
    return true;
}

bool Server::listen_socket(int server_fd, const ServerConfig &config)
{
    if (listen(server_fd, SOMAXCONN) == -1)
    {
        std::cerr << "Error: couldn't listen on " << config.address << ":" << config.port << ": " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "Success: listening on " << config.address << ":" << config.port << std::endl;
    return true;
}

// EPOLL
// Creiamo un'unica istanza epoll che controllerà
// tutti i listening socket e tutti i client.
bool Server::setup_epoll()
{
    this->epoll_fd = epoll_create(1);
    if (this->epoll_fd == -1)
    {
        std::cerr << "Error: epoll_create failed: " << strerror(errno) << std::endl;
        return false;
    }
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
bool Server::accept_client(int client_fd, size_t config_index)
{
    if (!set_nonblocking(client_fd))
        return false;

    if (!add_epoll_fd(client_fd, EPOLLIN))
        return false;

    this->clients[client_fd] = Client(client_fd);
    this->client_configs[client_fd] = config_index; // Ora, per ogni client, salviamo quale configurazione deve utilizzare

    return true;
}

void Server::close_client(int client_fd)
{
    if (this->epoll_fd != -1)
        epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);

    close(client_fd);

    this->clients.erase(client_fd);
    this->client_configs.erase(client_fd);
}

void Server::close_all_clients()
{
    while (!this->clients.empty())
    {
        close_client(this->clients.begin()->first);
    }
}

void Server::check_cgi_timeouts()
{
    time_t now = time(NULL);
    std::vector<int> expired;

    for (std::map<int, CgiProcess>::iterator it = this->cgi_processes.begin();
         it != this->cgi_processes.end(); ++it)
    {
        if (now - it->second.start >= CGI_TIMEOUT)
            expired.push_back(it->first);
    }

    for (size_t i = 0; i < expired.size(); ++i)
    {
        std::map<int, CgiProcess>::iterator it = this->cgi_processes.find(expired[i]);

        if (it == this->cgi_processes.end())
            continue;

        int client_fd = it->second.client_fd;
        pid_t pid = it->second.pid;

        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        close_cgi(expired[i]);

        std::map<int, Client>::iterator client_it = this->clients.find(client_fd);
        std::map<int, size_t>::iterator config_it = this->client_configs.find(client_fd);

        if (client_it == this->clients.end() || config_it == this->client_configs.end())
            continue;

        ServerConfig &config = this->configs[config_it->second];

        if (!client_it->second.prepare_error_response(504, config))
        {
            close_client(client_fd);
            continue;
        }

        if (!modify_epoll_fd(client_fd, EPOLLOUT))
            close_client(client_fd);
    }
}

void Server::close_cgi(int cgi_fd)
{
    std::map<int, CgiProcess>::iterator it = this->cgi_processes.find(cgi_fd);

    if (it == this->cgi_processes.end())
        return;

    if (this->epoll_fd != -1)
        epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, cgi_fd, NULL);

    close(cgi_fd);

    std::map<int, Client>::iterator client_it = this->clients.find(it->second.client_fd);

    if (client_it != this->clients.end())
        client_it->second.clear_cgi();

    this->cgi_processes.erase(it);
}

bool Server::handle_cgi_read(int cgi_fd)
{
    std::map<int, CgiProcess>::iterator it = this->cgi_processes.find(cgi_fd);

    if (it == this->cgi_processes.end())
        return false;

    CgiProcess &proc = it->second;
    int client_fd = proc.client_fd;

    std::map<int, Client>::iterator client_it = this->clients.find(client_fd);

    if (client_it == this->clients.end())
    {
        kill(proc.pid, SIGKILL);
        waitpid(proc.pid, NULL, 0);
        close_cgi(cgi_fd);
        return true;
    }

    Client &client = client_it->second;

    char buffer[SOCKET_BUFFER_SIZE];
    ssize_t bytes_read = read(cgi_fd, buffer, sizeof(buffer));

    if (bytes_read > 0)
    {
        proc.buffer.append(buffer, bytes_read);
        return true;
    }

    if (bytes_read < 0)
        return true;

    std::string output = proc.buffer;
    pid_t pid = proc.pid;

    waitpid(pid, NULL, WNOHANG);
    close_cgi(cgi_fd);

    std::map<int, size_t>::iterator config_it = this->client_configs.find(client_fd);

    if (config_it == this->client_configs.end() || config_it->second >= this->configs.size())
        return false;

    ServerConfig &config = this->configs[config_it->second];

    client.finish_cgi(output, config);

    if (DEBUG)
        client.print_response();

    if (!modify_epoll_fd(client_fd, EPOLLOUT))
        return false;

    return true;
}

bool Server::handle_client_read(int client_fd)
{
    std::map<int, Client>::iterator client_it = this->clients.find(client_fd);

    if (client_it == this->clients.end())
        return false;

    // Cerchiamo l'indice della configurazione
    // precedentemente associata a questo client.
    std::map<int, size_t>::iterator config_it = this->client_configs.find(client_fd);

    if (config_it == this->client_configs.end())
        return false;

    // Recuperiamo l'indice della ServerConfig.
    size_t config_index = config_it->second;

    if (config_index >= this->configs.size())
        return false;

    Client &client = client_it->second;
    // Recuperiamo la configurazione corretta per questo client.
    ServerConfig &config = this->configs[config_index];

    char temp[SOCKET_BUFFER_SIZE];

    ssize_t bytes_read = recv(client_fd, temp, sizeof(temp), 0);

    if (bytes_read <= 0)
    {
        std::cout << "recv failed and/or client disconnected: " << client_fd << std::endl;
        return false;
    }

    if (!client.has_full_headers(temp, bytes_read))
    {
        if (client.is_headers_too_large())
        {
            if (!client.prepare_error_response(431, config))
                return false;
            if (DEBUG)
                client.print_response();
            if (!modify_epoll_fd(client_fd, EPOLLOUT))
                return false;
            return true;
        }
        return true;
    }

    client.parse_request();

    if (client.req_error())
    {
        if (!client.prepare_error_response(400, config))
            return false;
        if (DEBUG)
            client.print_response();
        if (!modify_epoll_fd(client_fd, EPOLLOUT))
            return false;
        return true;
    }

    if (!client.req_done())
    {
        std::string cl = client.get_header("content-length");
        long content_length;

        if (!cl.empty() && string_to_long(cl, content_length) && static_cast<size_t>(content_length) > config.client_max_body_size)
        {
            if (!client.prepare_error_response(413, config))
                return false;
            if (!modify_epoll_fd(client_fd, EPOLLOUT))
                return false;
            return true;
        }
    }

    if (client.req_done())
    {
        if (DEBUG)
            client.print_request();

        if (!client.prepare_response(config))
            return false;

        int cgi_fd = client.get_cgi_fd();

        if (cgi_fd != -1)
        {
            if (!add_epoll_fd(cgi_fd, EPOLLIN))
            {
                kill(client.get_cgi_pid(), SIGKILL);
                waitpid(client.get_cgi_pid(), NULL, 0);
                close(cgi_fd);
                client.clear_cgi();
                return false;
            }

            CgiProcess proc;

            proc.client_fd = client_fd;
            proc.pid = client.get_cgi_pid();
            proc.start = time(NULL);

            this->cgi_processes[cgi_fd] = proc;

            return true;
        }

        if (DEBUG)
            client.print_response();

        if (!modify_epoll_fd(client_fd, EPOLLOUT))
            return false;
    }

    return true;
}

// bool Server::handle_client_read(int client_fd)
// {
//     std::map<int, Client>::iterator it = this->clients.find(client_fd);
//     if (it == this->clients.end())
//         return false;

//     Client &client = it->second; // client è un riferimento al Client dentro la map, quindi quando fai append modifichi davvero quel client.
//     char temp[SOCKET_BUFFER_SIZE];

//     ssize_t bytes_read = recv(client_fd, temp, sizeof(temp), 0);

//     if (bytes_read <= 0)
//     {
//         std::cout << "recv failed and/or Client disconnected: " << client_fd << std::endl;
//         return false;
//     }

//     if (!client.has_full_headers(temp, bytes_read))
//         return true;

//     client.parse_request();

//     if (client.req_error())
//     {
//         if (!client.prepare_error_response(400, this->config))
//             return false;
//         if (DEBUG)
//             client.print_response();
//         if (!modify_epoll_fd(client_fd, EPOLLOUT))
//             return false;
//         return true;
//     }

//     if (client.req_done())
//     {
//         if (DEBUG)
//             client.print_request();
//         if (!client.prepare_response(this->config))
//             return false;
//         if (DEBUG)
//             client.print_response();
//         if (!modify_epoll_fd(client_fd, EPOLLOUT))
//             return false;
//     }

//     return true;
// }

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
    this->running = true;
    epoll_event events[MAX_EPOLL_EVENTS];

    while (this->running)
    {
        int ready = epoll_wait(this->epoll_fd, events, MAX_EPOLL_EVENTS, EPOLL_TIMEOUT_MS);
        if (ready == -1)
        {
            if (errno == EINTR)
                continue;

            std::cerr << "Error: epoll_wait failed: " << strerror(errno) << std::endl;
            close_all_clients();
            return false;
        }

        check_cgi_timeouts();

        for (int i = 0; i < ready; ++i)
        {
            int current_fd = events[i].data.fd;
            uint32_t revents = events[i].events;

            std::map<int, size_t>::iterator listener_it = this->listening_sockets.find(current_fd);
            bool is_listening_socket = listener_it != this->listening_sockets.end();

            std::map<int, CgiProcess>::iterator cgi_it = this->cgi_processes.find(current_fd);
            bool is_cgi_pipe = cgi_it != this->cgi_processes.end();

            if (revents & (EPOLLERR | EPOLLHUP))
            {
                if (is_listening_socket)
                {
                    std::cerr << "Error: listening socket event failed" << std::endl;
                    close_all_clients();
                    return false;
                }

                if (is_cgi_pipe)
                {
                    if (!handle_cgi_read(current_fd))
                        close_cgi(current_fd);
                    continue;
                }

                close_client(current_fd);
                continue;
            }

            if ((revents & EPOLLIN) && is_listening_socket)
            {
                size_t config_index = listener_it->second;

                while (true)
                {
                    int client_fd = accept(current_fd, NULL, NULL);

                    if (client_fd == -1)
                    {
                        if (errno == EINTR)
                            continue;

                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;

                        std::cerr << "Error: accept failed: " << strerror(errno) << std::endl;
                        break;
                    }

                    if (!accept_client(client_fd, config_index))
                    {
                        close(client_fd);
                        continue;
                    }

                    std::cout << "Client connected, fd: " << client_fd << ", config: " << config_index << std::endl;
                }
                continue;
            }

            if ((revents & EPOLLIN) && is_cgi_pipe)
            {
                if (!handle_cgi_read(current_fd))
                    close_cgi(current_fd);
                continue;
            }

            if ((revents & EPOLLIN) && !is_listening_socket)
            {
                if (!handle_client_read(current_fd))
                {
                    close_client(current_fd);
                    continue;
                }
            }

            if ((revents & EPOLLOUT) && !is_listening_socket)
            {
                if (!handle_client_write(current_fd))
                {
                    close_client(current_fd);
                    continue;
                }
            }
        }
    }

    close_all_clients();
    return true;
}

// bool Server::run()
// {
//     int client_fd;

//     this->running = true;
//     epoll_event events[MAX_EPOLL_EVENTS];

//     // DEBUG: remove after testing
//     // time_t start = time(NULL);
//     while (this->running)
//     {
//         // DEBUG: stoppo il server dopo 5 secondi per non doverlo killare ogni volta.
//         // if ((DEBUG) && (time(NULL) - start >= 5))
//         //     this->running = false;

//         int ready = epoll_wait(this->epoll_fd, events, MAX_EPOLL_EVENTS, -1);
//         if (ready == -1)
//         {
//             if (errno == EINTR)
//                 continue;
//             std::cerr << "Error: epoll_wait failed: " << strerror(errno) << std::endl;
//             close_all_clients();
//             return false;
//         }
//         for (int i = 0; i < ready; ++i)
//         {
//             int current_fd = events[i].data.fd;
//             if (current_fd == this->fd && (revents & (EPOLLERR | EPOLLHUP)))
//             {
//                 std::cerr << "Error: server socket epoll event failed" << std::endl;
//                 close_all_clients();
//                 return false;
//             }
//             if (current_fd != this->fd && (revents & (EPOLLERR | EPOLLHUP)))
//             {
//                 close_client(current_fd);
//                 continue;
//             }
//             if (revents & EPOLLIN)
//             {
//                 if (current_fd == this->fd)
//                 {
//                     client_fd = accept(this->fd, NULL, NULL);
//                     if (client_fd == -1)
//                     {
//                         std::cerr << "Error: accept failed: " << strerror(errno) << std::endl;
//                         continue;
//                     }
//                     if (!accept_client(client_fd))
//                     {
//                         close(client_fd);
//                         continue;
//                     }
//                     std::cout << "Client connected, fd: " << client_fd << "\n";
//                 }
//                 else
//                 {
//                     if (!handle_client_read(current_fd)) // DA IMPLEMENTARE
//                     {
//                         close_client(current_fd);
//                         continue;
//                     }
//                 }
//             }
//
//             if (revents & EPOLLOUT)
//             {
//                 if (current_fd != this->fd)
//                 {
//                     if (!handle_client_write(current_fd))
//                         close_client(current_fd);
//                     continue;
//                 }
//             }
//         }
//     }
//     close_all_clients();
//     return true;
// }
bool Server::setup(const std::vector<ServerConfig> &parsed_configs)
{
    if (parsed_configs.empty())
    {
        std::cerr << "Error: no server configurations provided" << std::endl;
        return false;
    }

    this->configs = parsed_configs;

    if (!setup_epoll())
        return false;

    for (size_t i = 0; i < this->configs.size(); ++i)
    {
        const ServerConfig &config = this->configs[i];

        int server_fd = create_socket();

        if (server_fd == -1)
            return false;

        if (!set_nonblocking(server_fd))
        {
            std::cerr << "Error: couldn't set server socket " << server_fd << " as non-blocking" << std::endl;
            close(server_fd);
            return false;
        }

        int opt = 1;

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        {
            std::cerr << "Error: couldn't set SO_REUSEADDR on " << config.address << ":" << config.port << ": " << strerror(errno) << std::endl;
            close(server_fd);
            return false;
        }

        if (!bind_socket(server_fd, config))
        {
            close(server_fd);
            return false;
        }

        if (!listen_socket(server_fd, config))
        {
            close(server_fd);
            return false;
        }

        if (!add_epoll_fd(server_fd, EPOLLIN))
        {
            close(server_fd);
            return false;
        }

        this->listening_sockets[server_fd] = i; // Salva l’associazione: socket di ascolto → indice della configurazione
    }

    return true;
}
