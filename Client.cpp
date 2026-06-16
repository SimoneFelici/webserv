#include "Client.hpp"

Client::Client(int fd) : fd(fd) {}

Client::Client() : fd(-1) {}

Client::Client(const Client &other) : fd(other.fd) {}

// il distruttore per ora non chiude il fd.
Client::~Client() {}

Client &Client::operator=(const Client &other)
{
    if (this != &other)
        this->fd = other.fd;
    return *this;
}

int Client::get_fd() const { return this->fd; }

void Client::set_fd(int fd) { this->fd = fd; }
