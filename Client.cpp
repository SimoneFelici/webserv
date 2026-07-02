#include "Client.hpp"
#include "Server.hpp"
#include "webserv.hpp"

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

bool Client::prepare_error_response(int error_code)
{
    if (!this->clear_response())
        return false;

    if (this->req.version.empty())
        this->req.version = "HTTP/1.1";
    build_error_response(error_code);
    build_response_buffer();
    return true;
}

void Client::print_request() const
{
    std::cout << "\nMethod: " << req.method << std::endl;
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
    this->req = HttpRequest();
    // this->response_buffer.clear(); // deve gestirli la clear responpse senno è "sporco"
    // this->bytes_sent = 0;
}

bool Client::req_done() const
{
    return (req.state == HttpRequest::DONE);
}

bool Client::req_error() const
{
    return req.state == HttpRequest::ERROR;
}

bool Client::parse_request_line(std::size_t &pos)
{
    size_t end = this->request_buffer.find("\r\n");
    std::string line = this->request_buffer.substr(0, end);
    size_t first_space = line.find(' ');
    size_t second_space = line.find(' ', first_space + 1);

    if (first_space == std::string::npos || second_space == std::string::npos)
    {
        req.state = HttpRequest::ERROR;
        return true;
    }

    if (line.find(' ', second_space + 1) != std::string::npos)
    {
        req.state = HttpRequest::ERROR;
        return true;
    }

    req.method = line.substr(0, first_space);
    req.path = line.substr(first_space + 1, second_space - first_space - 1);
    req.version = line.substr(second_space + 1);

    if (req.method.empty() || req.path.empty() || req.path[0] != '/')
    {
        req.state = HttpRequest::ERROR;
        return true;
    }

    if (req.version.compare(0, 5, "HTTP/") != 0)
    {
        req.state = HttpRequest::ERROR;
        return true;
    }

    pos = end + 2;
    req.state = HttpRequest::PARSING_HEADERS;
    return true;
}

bool Client::parse_headers(std::size_t &pos)
{
    size_t headers_end = this->request_buffer.find("\r\n\r\n");
    size_t line_start = pos;
    while (line_start < headers_end)
    {
        size_t line_end = this->request_buffer.find("\r\n", line_start);
        if (line_end == std::string::npos || line_end > headers_end)
        {
            req.state = HttpRequest::ERROR;
            return true;
        }
        std::string line = this->request_buffer.substr(line_start, line_end - line_start);
        size_t colon = line.find(':');
        if (colon == std::string::npos || colon == 0)
        {
            req.state = HttpRequest::ERROR;
            return true;
        }
        std::string key = line.substr(0, colon);
        size_t value_start = colon + 1;
        while (value_start < line.size() && line[value_start] == ' ')
            ++value_start;
        std::string value = line.substr(value_start);

        req.headers[key] = value;
        line_start = line_end + 2;
    }
    pos = headers_end + 4;
    req.body_start = pos;
    req.state = HttpRequest::PARSING_BODY;
    return true;
}

bool Client::parse_body(std::size_t &pos)
{
    if (req.headers.count("Content-Length"))
    {
        const std::string &cl = req.headers["Content-Length"];
        for (size_t i = 0; i < cl.size(); ++i)
        {
            if (!isdigit(static_cast<unsigned char>(cl[i])))
            {
                req.state = HttpRequest::ERROR;
                return true;
            }
        }
        long content_length = std::atol(cl.c_str());
        if (content_length < 0)
        {
            req.state = HttpRequest::ERROR;
            return true;
        }
        size_t available = this->request_buffer.size() - pos;
        if (available < static_cast<size_t>(content_length))
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
    if (req.state == HttpRequest::PARSING_BODY)
        pos = req.body_start;

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
        case HttpRequest::ERROR:
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

std::string Client::get_header(const std::string &key) const
{
    std::map<std::string, std::string>::const_iterator it = this->req.headers.find(key);
    if (it == this->req.headers.end())
        return "";
    return it->second;
}

const std::string &Client::get_response() const
{
    return this->response_buffer;
}

std::size_t Client::get_bytes_sent() const
{
    return this->bytes_sent;
}

void Client::add_bytes_sent(std::size_t bytes)
{
    this->bytes_sent += bytes;
}

bool Client::clear_response()
{
    if (this->bytes_sent < this->response_buffer.size())
        return false;

    this->response_buffer.clear();
    this->bytes_sent = 0;
    return true;
}

void Client::build_response_buffer()
{
    std::stringstream ss;

    this->res.version = this->get_version(); // verrà gestito prima durante la validazione della request
    ss << this->res.version << " " << this->res.status_code << " " << this->res.reason << "\r\n";
    ss << "Content-Type: " << this->res.content_type << "\r\n";
    ss << "Content-Length: " << this->res.body.size() << "\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n";
    ss << this->res.body;

    this->response_buffer = ss.str();
    this->bytes_sent = 0;
}

void Client::build_error_response(int error_code)
{
    std::string reason;
    std::map<int, std::string> Errorcodes;
    Errorcodes[400] = "Bad Request";
    Errorcodes[403] = "Forbidden";
    Errorcodes[404] = "Not Found";
    Errorcodes[405] = "Method Not Allowed";
    Errorcodes[413] = "Payload Too Large";
    Errorcodes[500] = "Internal Server Error";
    Errorcodes[505] = "HTTP Version Not Supported";

    std::map<int, std::string>::iterator it = Errorcodes.find(error_code);
    if (it != Errorcodes.end())
    {
        reason = it->second;
    }
    else
    {
        error_code = 500;
        reason = "Internal Server Error";
    }

    this->res.status_code = error_code;
    this->res.reason = reason;
    this->res.content_type = "text/html";

    std::stringstream body;
    body << "<html><body><h1>" << error_code << " " << reason << "</h1></body></html>";

    this->res.body = body.str();
}

bool Client::handle_get_req(ServerConfig &config, const LocationConfig *loc)
{
    std::string root;
    std::string index;

    if (loc && !loc->root.empty())
        root = loc->root;
    else
        root = config.root;

    if (loc && !loc->index.empty())
        index = loc->index;
    else
        index = config.index;

    std::string file_path;

    this->res.reason = "OK";
    this->res.content_type = "text/html";

    if (this->get_path() == "/" || (loc && this->get_path() == loc->path))
        file_path = root + "/" + index;
    else
        file_path = root + this->get_path();

    this->res.status_code = read_file(file_path, this->res.body);
    if (this->res.status_code != 200)
        build_error_response(this->res.status_code);

    return true;
}

const LocationConfig *Client::match_location(const ServerConfig &config) const
{
    const LocationConfig *best = NULL;
    size_t best_len = 0;
    const std::string &path = this->get_path();

    for (size_t i = 0; i < config.locations.size(); ++i)
    {
        const LocationConfig &l = config.locations[i];
        if (l.path.empty())
            continue;
        if (path.compare(0, l.path.size(), l.path) == 0 && l.path.size() > best_len)
        {
            best = &l;
            best_len = l.path.size();
        }
    }
    return best;
}

bool Client::is_method_allowed(const std::vector<std::string> &allowed) const
{
    for (size_t i = 0; i < allowed.size(); ++i)
        if (allowed[i] == this->get_method())
            return true;
    return false;
}

int Client::validate_req(ServerConfig &config, const LocationConfig *&loc)
{
    if (this->get_version() != "HTTP/1.1")
        return 505;

    loc = match_location(config);

    const std::vector<std::string> *allowed;
    if (loc && !loc->allowed_methods.empty())
        allowed = &loc->allowed_methods;
    else
        allowed = &config.allowed_methods;

    if (!is_method_allowed(*allowed))
        return 405;

    // TODO: Content-Length / body size limit -> 413

    return 0;
}

bool Client::prepare_response(ServerConfig &config)
{

    if (!this->clear_response())
        return false;

    const LocationConfig *loc = NULL;
    int status = validate_req(config, loc);

    if (status != 0)
    {
        build_error_response(status);
        build_response_buffer();
        return true;
    }

    if (this->get_method() == "GET")
        handle_get_req(config, loc);
    // else if (this->get_method() == "POST")
    //     handle_post(config);
    // else if (this->get_method() == "DELETE")
    //     handle_delete(config);
    else
        build_error_response(501);

    build_response_buffer();
    return true;
}
