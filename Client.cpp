#include "Client.hpp"

Client::Client(int fd) : client_fd(fd), _bytes_sent(0) {}

Client::Client() : client_fd(-1), _bytes_sent(0) {}

// il distruttore per ora non chiude il fd.
Client::~Client() {}

int Client::get_fd() const
{
    return this->client_fd;
}

void Client::set_fd(int fd)
{
    this->client_fd = fd;
}

const std::string &Client::get_request() const
{
    return this->request_buffer;
}

bool Client::has_full_headers() const
{
    return this->request_buffer.find("\r\n\r\n") != std::string::npos;
}

bool Client::has_full_header(const char *data, size_t len) 
{
    this->request_buffer.append(data, len); // append() aggiunge roba alla fine di una std::string già esistente.
    return this->has_full_headers();
}

void Client::clear_request()
{
    this->request_buffer.clear();
    this->response_buffer.clear();
    this->_bytes_sent = 0;
}

bool Client::prepare_response(std::string response)
{
    this->response_buffer = response;
    return true;
}
