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
    this->req = HttpRequest();
    // this->response_buffer.clear(); // deve gestirli la clear responpse senno è "sporco"
    // this->bytes_sent = 0;
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
    req.version = line.substr(second_space + 1);
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

bool Client::handle_get_req(ServerConfig &config)
{

    std::string file_path;

    this->res.reason = "OK";
    this->res.content_type = "text/html";

    if (this->get_path() == "/")
        file_path = config.root + "/" + config.index;
    else
        file_path = config.root + this->get_path();
    this->res.status_code = read_file(file_path, this->res.body);
    if (this->res.status_code != 200)
        build_error_response(this->res.status_code);

    return true;
}

bool Client::is_allowed_method(ServerConfig &config)
{
    std::string path = this->get_path();     // mi prendo il path della request
    std::string method = this->get_method(); // mi prendo il method della request

    const LocationConfig *bestMatch = NULL; // imposto a null perche potrebbero non esserci match
    size_t best_len = 0;                    // devo controllare anche la corrispondenza per lunghezza

    // inizio a scorrere le location dentro la config
    for (std::vector<LocationConfig>::const_iterator it = config.locations.begin();
         it != config.locations.end();
         ++it)
    {
        const std::string &location_path = it->_location; // mi prendo la location

        if (location_path.empty()) // check di protezione: se è vuota vado alla prossima
            continue;

        /*
            La location matcha se:

            1. path == location_path
               esempio:
               path = "/uploads"
               location_path = "/uploads"

            oppure

            2. la path inizia con location_path
               esempio:
               path = "/uploads/file.txt"
               location_path = "/uploads"

               Però controllo anche che dopo "/uploads" ci sia "/",
               così evito che "/uploads" matchi erroneamente "/uploads2".
        */
        if (path == location_path || (path.find(location_path) == 0 && (location_path == "/" || path[location_path.size()] == '/')))
        {
            if (location_path.size() > best_len)
            {
                bestMatch = &(*it);              // salvo il puntatore a tutta la LocationConfig corrente
                best_len = location_path.size(); // salvo la lunghezza del match migliore
            }
        }
    }
    if (bestMatch == NULL)
    {
        std::cout << "DEBUG: nessuna location matchata" << std::endl;
        return false;
    }

    std::cout << "DEBUG: location matchata: " << bestMatch->_location << std::endl;

    /*
    1. prende il path della request dal client
    2. cerca la location migliore dentro config.locations
    3. se non trova nessuna location -> false oppure fallback
    4. prende allowed_methods di quella location
    5. controlla se this->get_method() è dentro allowed_methods
    6. se sì ritorna true
    7. se no ritorna false
    */
    return true;
}

/*
1. pulisco response
1.1 Validazione response
2. trovo la location migliore per req.path
3. se non trovo location: uso fallback oppure errore
4. controllo se req.method è dentro location.allowed_methods
5. se non è consentito: 405 Method Not Allowed
6. se è consentito:
       if GET -> handle_get_req(location/server config)
       if POST -> handle_post_req(...)
       if DELETE -> handle_delete_req(...)
7. build_response_buffer()
*/
bool Client::prepare_response(ServerConfig &config)
{

    if (!this->clear_response())
        return false;
    // QUI SI PUUO INSERIRE VALIDAZIONE

    // if (!is_allowed_method(config))
    //     // build_error_response(405);
    // is_allowed_method(config);

    // QUI VA MESSO CHECK DEI METODI CONSENTITI

    if (this->get_method() == "GET")
        handle_get_req(config);
    // else if (this->get_method() == "POST")
    //     handle_post(config);
    // else if (this->get_method() == "DELETE")
    //     handle_delete(config);
    else
        build_error_response(405);

    build_response_buffer();
    return true;
}
