#include "Client.hpp"

Client::Client(int fd) : fd(fd), _bytes_sent(0) {}

Client::Client() : fd(-1), _bytes_sent(0) {}

// il distruttore per ora non chiude il fd.
Client::~Client() {}

int Client::get_fd() const
{
    return this->fd;
}

void Client::set_fd(int fd)
{
    this->fd = fd;
}
