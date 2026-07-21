#include "Client.hpp"
#include "Config.hpp"
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

bool Client::prepare_error_response(int error_code, const ServerConfig &config)
{
    if (!this->clear_response())
        return false;

    this->res.headers.clear();

    if (this->req.version.empty())
        this->req.version = "HTTP/1.1";

    build_error_response(error_code, config, NULL);
    build_response_buffer();

    return true;
}

void Client::print_request() const
{
    std::cout << "\n--- REQUEST ---" << std::endl;
    std::cout << "Method: " << req.method << std::endl;
    std::cout << "Path: " << req.path << std::endl;
    std::cout << "Version: " << req.version << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it)
        std::cout << it->first << ": " << it->second << std::endl;
    if (!req.body.empty())
        std::cout << "Body: " << req.body << std::endl;
    std::cout << "---------" << std::endl;
}

void Client::print_response() const
{
    std::cout << "\n--- RESPONSE ---" << std::endl;
    std::cout << this->response_buffer << std::endl;
    std::cout << "---------" << std::endl;
}

// TODO: maybe add a limit?
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

bool Client::parse_header_line(const std::string &line)
{
    size_t colon = line.find(':');
    if (colon == std::string::npos || colon == 0)
        return false;

    std::string key = line.substr(0, colon);

    if (key.find_first_of(" \t") != std::string::npos)
        return false;

    to_lower(key);
    req.headers[key] = trim(line.substr(colon + 1));
    return true;
}

bool Client::parse_headers(std::size_t &pos)
{
    size_t headers_end = this->request_buffer.find("\r\n\r\n");
    size_t line_start = pos;

    while (line_start < headers_end)
    {
        size_t line_end = this->request_buffer.find("\r\n", line_start);
        std::string line = this->request_buffer.substr(line_start, line_end - line_start);

        if (!parse_header_line(line))
        {
            req.state = HttpRequest::ERROR;
            return true;
        }
        line_start = line_end + 2;
    }

    if (req.headers.count("host") == 0)
    {
        req.state = HttpRequest::ERROR;
        return true;
    }

    pos = headers_end + 4;
    req.body_start = pos;
    req.state = HttpRequest::PARSING_BODY;
    return true;
}

bool Client::parse_body(std::size_t &pos)
{
    if (req.headers.count("content-length"))
    {
        const std::string &cl = req.headers["content-length"];

        // TODO: For now we can parse a max of 1GB
        if (cl.empty() || cl.size() > 9)
        {
            req.state = HttpRequest::ERROR;
            return true;
        }
        for (size_t i = 0; i < cl.size(); ++i)
        {
            if (!isdigit(static_cast<unsigned char>(cl[i])))
            {
                req.state = HttpRequest::ERROR;
                return true;
            }
        }
        long content_length = std::atol(cl.c_str());
        // TODO: when parsing client_max_body_size add check

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
    std::string lower(key);
    to_lower(lower);

    std::map<std::string, std::string>::const_iterator it = this->req.headers.find(lower);
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

    this->res.version = "HTTP/1.1";
    ss << this->res.version << " " << this->res.status_code << " " << this->res.reason << "\r\n";
    ss << "Content-Type: " << this->res.content_type << "\r\n";
    ss << "Content-Length: " << this->res.body.size() << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = this->res.headers.begin();
         it != this->res.headers.end(); ++it)
        ss << it->first << ": " << it->second << "\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n";
    ss << this->res.body;

    this->response_buffer = ss.str();
    this->bytes_sent = 0;
}

void Client::build_default_error_response(int error_code)
{
    std::string reason = get_error_reason(error_code);

    if (reason == "Internal Server Error" && error_code != 500)
    {
        error_code = 500;
        reason = "Internal Server Error";
    }

    this->res.status_code = error_code;
    this->res.reason = reason;
    this->res.content_type = "text/html";

    std::stringstream body;

    body << "<html>" << "<head><title>" << error_code << " " << reason << "</title></head>" << "<body>" << "<h1>" << error_code << " " << reason << "</h1>" << "</body>" << "</html>";

    this->res.body = body.str();
}

std::string Client::get_error_reason(int error_code) const
{
    std::map<int, std::string> error_codes;

    error_codes[400] = "Bad Request";
    error_codes[403] = "Forbidden";
    error_codes[404] = "Not Found";
    error_codes[405] = "Method Not Allowed";
    error_codes[413] = "Payload Too Large";
    error_codes[500] = "Internal Server Error";
    error_codes[501] = "Not Implemented";
    error_codes[502] = "Internal Server Error";
    error_codes[505] = "HTTP Version Not Supported";

    std::map<int, std::string>::const_iterator it = error_codes.find(error_code);

    if (it == error_codes.end())
        it = error_codes.find(500);

    return it->second;
}
void Client::build_error_response(int error_code, const ServerConfig &config, const LocationConfig *loc)
{
    std::string error_page_path;
    std::map<int, std::string>::const_iterator it;

    /* Prima controlliamo la location più specifica.*/
    if (loc)
    {
        it = loc->error_pages.find(error_code);

        if (it != loc->error_pages.end())
            error_page_path = it->second;
    }

    /*Se la location non ha una pagina per questo errore, controlliamo la configurazione del server.*/
    if (error_page_path.empty())
    {
        it = config.error_pages.find(error_code);

        if (it != config.error_pages.end())
            error_page_path = it->second;
    }

    /*Se è stata configurata una pagina, proviamo a leggerla.*/
    if (!error_page_path.empty())
    {
        std::string body;
        int read_status = read_file(error_page_path, body);

        if (read_status == 200)
        {
            this->res.status_code = error_code;
            this->res.reason = get_error_reason(error_code);
            this->res.content_type = get_content_type(error_page_path);
            this->res.body = body;
            return;
        }
    }

    /* Nessuna pagina configurata, oppure file non leggibile: usiamo la pagina HTML interna. */
    build_default_error_response(error_code);
}

/* prende una stringa e la rende sicura da stampare dentro una pagina HTML. Se trova caratteri speciali HTML, li sostituisce. */
static std::string html_escape(const std::string &s)
{
    std::string escaped;

    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '&')
            escaped += "&amp;";
        else if (s[i] == '<')
            escaped += "&lt;";
        else if (s[i] == '>')
            escaped += "&gt;";
        else if (s[i] == '"')
            escaped += "&quot;";
        else
            escaped += s[i];
    }
    return escaped;
}
/* Questa funzione serve solo nell’autoindex, dentro build_autoindex_body(), serve a costruire l’href.
 */
static std::string join_url_path(const std::string &base, const std::string &name)
{
    if (base.empty())
        return "/" + name;
    if (base[base.size() - 1] == '/')
        return base + name;
    return base + "/" + name;
}

static int build_autoindex_body(const std::string &dir_path, const std::string &request_path, std::string &body)
{
    /*
    DIR è un tipo della libreria C/POSIX, dichiarato in: #include <dirent.h>
    Serve a rappresentare una directory aperta.
    */
    DIR *dir = opendir(dir_path.c_str()); // apri questa cartella, dammi un puntatore/handle per leggerne il contenuto

    if (!dir)
    {
        if (errno == EACCES)
            return 403;
        return 500;
    }

    std::stringstream ss;
    ss << "<html><body><h1>Index of " << html_escape(request_path) << "</h1><ul>";

    struct dirent *entry;                  // Poi con quel dir puoi leggere le entries:
    while ((entry = readdir(dir)) != NULL) // Legge una voce alla volta dalla directory.
    {
        std::string name = entry->d_name;

        if (name == ".")
            continue;
        /*
        html_escape(request_path) serve a trasformare alcuni caratteri speciali in testo “sicuro” da mettere dentro HTML.
        Se request_path contiene caratteri tipo < o >, il browser può interpretarli come tag HTML.
        */
        ss << "<li><a href=\"" << html_escape(join_url_path(request_path, name)) << "\">" << html_escape(name) << "</a></li>";
    }

    closedir(dir); // Chiude la directory aperta con opendir().
    ss << "</ul></body></html>";
    body = ss.str();
    return 200;
}

std::string Client::build_file_path(const ServerConfig &config, const LocationConfig *loc) const
{
    std::string root;

    if (loc && !loc->root.empty())
        root = loc->root;
    else
        root = config.root;

    std::string relative_path = this->get_path();

    if (loc && loc->path != "/" &&
        relative_path.compare(0, loc->path.size(), loc->path) == 0)
    {
        relative_path = relative_path.substr(loc->path.size());
    }

    if (relative_path.empty())
        relative_path = "/";

    if (relative_path == "/")
        return root;

    return root + relative_path;
}

bool Client::handle_get_req(ServerConfig &config, const LocationConfig *loc)
{
    std::string file_path; // file_path: sarà il path reale sul filesystem.
    std::string directory_path;
    std::string index;     // index: nome del file index, tipo index.html.
    struct stat file_stat; // file_stat: struct riempita da stat() per capire se il path esiste, se è file, directory, ecc.

    file_path = build_file_path(config, loc); // costruisce il path reale. Qui trasforma l’URL richiesto in path filesystem.

    // Controlla se il path esiste: stat() prova a leggere informazioni sul path.
    if (stat(file_path.c_str(), &file_stat) == -1)
    {
        if (errno == EACCES) // non ho permessi -> 403
            build_error_response(403, config, loc);
        else if (errno == ENOENT || errno == ENOTDIR) // non esiste o un pezzo del path non è directory
            build_error_response(404, config, loc);
        else
            build_error_response(500, config, loc); // errore interno
        return true;
    }

    if (S_ISDIR(file_stat.st_mode)) // Se è una directory, prova a cercare index
    {
        directory_path = file_path;
        // allora sceglie il nome dell’index: Quindi usa prima l’index della location, se esiste; altrimenti quello globale.
        if (loc && !loc->index.empty())
            index = loc->index;
        else
            index = config.index;

        if (file_path.empty() || file_path[file_path.size() - 1] != '/')
            file_path += "/";
        file_path += index;

        // Controlla se l’index esiste
        if (stat(file_path.c_str(), &file_stat) == -1)
        {
            if (errno == EACCES) // Se non puoi accedere all’index: 403
                build_error_response(403, config, loc);
            else if (errno == ENOENT || errno == ENOTDIR) // Se l’index non esiste, Sceglie se autoindex è attivo:
            {
                /*
                se c’è una location -> usa loc->autoindex
                altrimenti -> usa config.autoindex
                */
                int autoindex_value = config.autoindex;
                if (loc && loc->autoindex != -1)
                    autoindex_value = loc->autoindex;
                if (autoindex_value != 1) // Se autoindex è off 403
                    build_error_response(403, config, loc);
                else
                {
                    // Se autoindex è on, genera il body HTML della directory listing.
                    this->res.status_code = build_autoindex_body(directory_path, this->get_path(), this->res.body);
                    if (this->res.status_code != 200)
                        build_error_response(this->res.status_code, config, loc);
                    else
                    {
                        this->res.reason = "OK";
                        this->res.content_type = "text/html";
                    }
                }
            }
            else
                build_error_response(500, config, loc);
            return true;
        }
    }

    // Verifica che il path finale sia un file normale
    if (!S_ISREG(file_stat.st_mode))
    {
        build_error_response(403, config, loc);
        return true;
    }

    // Prepara risposta 200
    this->res.reason = "OK";
    this->res.content_type = get_content_type(file_path);

    // Legge il file apre il file, legge il contenuto e lo mette in res.body.
    /*
    200 -> letto correttamente
    403 -> permessi negati
    404 -> file non trovato
    500 -> errore interno
    */
    this->res.status_code = read_file(file_path, this->res.body);
    // Se la lettura fallisce, costruisce errore
    if (this->res.status_code != 200)
        build_error_response(this->res.status_code, config, loc);

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
        if (path.compare(0, l.path.size(), l.path) != 0)
            continue;
        if (l.path != "/" && path.size() > l.path.size() && path[l.path.size()] != '/')
            continue;
        if (l.path.size() > best_len)
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

// TODO: to check implementation
int Client::sanitize_path()
{
    std::string &path = req.path;
    std::vector<std::string> segs;

    size_t i = 1; // path[0] è '/' garantito da parse_request_line
    while (i <= path.size())
    {
        size_t j = path.find('/', i);
        if (j == std::string::npos)
            j = path.size();
        std::string seg = path.substr(i, j - i);

        if (seg == "..")
        {
            if (segs.empty())
                return 403; // uscirebbe sopra la root
            segs.pop_back();
        }
        else if (!seg.empty() && seg != ".")
            segs.push_back(seg);
        i = j + 1;
    }

    bool trailing_slash = path.size() > 1 && path[path.size() - 1] == '/';
    path = "/";
    for (size_t k = 0; k < segs.size(); ++k)
    {
        path += segs[k];
        if (k + 1 < segs.size())
            path += "/";
    }
    if (trailing_slash && path != "/")
        path += "/";
    return 0;
}
int Client::validate_req(ServerConfig &config, const LocationConfig *&loc)
{
    if (this->get_version() != "HTTP/1.1")
        return 505;

    int status = sanitize_path();
    if (status != 0)
        return status;

    loc = match_location(config);

    const std::vector<std::string> *allowed;

    if (loc && !loc->allowed_methods.empty())
        allowed = &loc->allowed_methods;
    else
        allowed = &config.allowed_methods;

    if (!is_method_allowed(*allowed))
    {
        std::string allow;

        for (size_t i = 0; i < allowed->size(); ++i)
        {
            if (i > 0)
                allow += ", ";
            allow += (*allowed)[i];
        }

        this->res.headers["Allow"] = allow;
        return 405;
    }

    if (this->req.headers.count("content-length"))
    {
        long content_length;

        if (!string_to_long(
                this->req.headers["content-length"],
                content_length))
            return 400;

        if (content_length < 0)
            return 400;

        if (static_cast<size_t>(content_length) >
            config.client_max_body_size)
            return 413;
    }

    return 0;
}

bool Client::prepare_response(ServerConfig &config)
{

    if (!this->clear_response())
        return false;

    this->res.headers.clear();
    const LocationConfig *loc = NULL;
    int status = validate_req(config, loc);

    if (status != 0)
    {
        build_error_response(status, config, loc);
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
        build_error_response(501, config, loc);

    build_response_buffer();
    return true;
}
