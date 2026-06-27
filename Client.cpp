#include "Client.hpp"

Client::Client(int fd) : client_fd(fd), bytes_sent(0) {}

Client::Client() : client_fd(-1), bytes_sent(0) {}

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

void Client::print_request() const
{
    std::cout << "Method: " << req.method << std::endl;
    std::cout << "Path: " << req.path << std::endl;
    std::cout << "Version: " << req.version << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it)
        std::cout << it->first << ": " << it->second << std::endl;
    if (!req.body.empty())
        std::cout << "Body: " << req.body << std::endl;
}

bool Client::has_full_headers(const char *data, size_t len)
{
    this->request_buffer.append(data, len);
    return this->request_buffer.find("\r\n\r\n") != std::string::npos;
}

void Client::clear_request()
{
    this->request_buffer.clear();
    this->response_buffer.clear();
    this->bytes_sent = 0;
    this->req = HttpRequest();
}

bool Client::req_done() const
{
    return (req.state == HttpRequest::DONE);
}

bool Client::parse_request_line(std::size_t &pos)
{
    size_t end = this->request_buffer.find("\r\n");
    if (end == std::string::npos)
        return false;
    std::string line = this->request_buffer.substr(0, end);
    size_t first_space = line.find(' ');
    size_t second_space = line.find(' ', first_space + 1);

    req.method = line.substr(0, first_space);
    req.path = line.substr(first_space + 1, second_space - first_space - 1);
    // Qui non è proprio corretto. Il secondo parametro di substr è una lunghezza, non “fino a \r”. 
    //Siccome line è già senza \r\n, basta: req.version = line.substr(second_space + 1);
    req.version = line.substr(second_space + 1, '\r');
    pos = end + 2;
    req.state = HttpRequest::PARSING_HEADERS;
    return true;
}

bool Client::parse_headers(std::size_t &pos)
{
    size_t headers_end = this->request_buffer.find("\r\n\r\n");
    if (headers_end == std::string::npos)
        return false;
    size_t line_start = pos;
    while (line_start < headers_end)
    {
        size_t line_end = this->request_buffer.find("\r\n", line_start);
        std::string line = this->request_buffer.substr(line_start, line_end - line_start);
        size_t colon = line.find(':');
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 2);
        req.headers[key] = value;
        line_start = line_end + 2;
    }
    pos = headers_end + 4;
    req.state = HttpRequest::PARSING_BODY;
    return true;
}

bool Client::parse_body(std::size_t &pos)
{
    if (req.headers.count("Content-Length"))
    {
        size_t content_length = std::atoi(req.headers["Content-Length"].c_str());
        size_t available = this->request_buffer.size() - pos;
        if (available < content_length)
            return true;
        req.body = this->request_buffer.substr(pos, content_length);
        pos += content_length;
    }
    req.state = HttpRequest::DONE;
    return true;
}

bool Client::parse_request()
{
    std::size_t pos = 0;
    while (pos < this->request_buffer.size() || req.state == HttpRequest::PARSING_BODY)
    {
        switch (req.state)
        {
        case HttpRequest::PARSING_REQUEST_LINE:
            if (!parse_request_line(pos))
                return false;
            break;
        case HttpRequest::PARSING_HEADERS:
            if (!parse_headers(pos))
                return false;
            break;
        case HttpRequest::PARSING_BODY:
            return parse_body(pos);
        case HttpRequest::DONE:
            return true;
        }
    }
    return false;
}

const std::string &Client::get_method() const 
{
    return this->req.method;
}

const std::string &Client::get_path() const
{
    return this->req.path;
}

const std::string &Client::get_version() const
{
    return this->req.version;
}

const std::string &Client::get_body() const
{
    return this->req.body;
}

const std::string &Client::get_header(const std::string &key) const
{
    std::map<std::string, std::string>::const_iterator it = this->req.headers.find(key);
    if (it == his->req.headers.end())
        return "";
    return it->second;
}