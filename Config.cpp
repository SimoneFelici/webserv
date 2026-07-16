#include "Config.hpp"
#include "Server.hpp"

LocationConfig::LocationConfig() : autoindex(-1), redirect_code(0)
{
}

ServerConfig::ServerConfig() : autoindex(-1), max_conn(0), client_max_body_size(0)
{
}

Config::Config() {}

Config::~Config() {}

const std::vector<ServerConfig> &Config::getConfigs() const
{
    return configs;
}

void Config::printConfig() const
{
    std::cout << "\n========== CONFIG ==========" << std::endl;
    for (size_t i = 0; i < configs.size(); ++i)
    {
        const ServerConfig &server = configs[i];
        std::cout << "\nSERVER " << i + 1 << std::endl;
        std::cout << "address: " << server.address << std::endl;
        std::cout << "port: " << server.port << std::endl;
        std::cout << "server_name: " << server.server_name << std::endl;
        std::cout << "root: " << server.root << std::endl;
        std::cout << "index: " << server.index << std::endl;
        std::cout << "autoindex: " << server.autoindex << std::endl;
        std::cout << "client_max_body_size: " << server.client_max_body_size << std::endl;
        std::cout << "allowed_methods:";
        for (size_t j = 0; j < server.allowed_methods.size(); ++j)
            std::cout << " " << server.allowed_methods[j];
        std::cout << std::endl;
        std::cout << "error_pages:" << std::endl;
        for (std::map<int, std::string>::const_iterator it = server.error_pages.begin(); it != server.error_pages.end(); ++it)
            std::cout << "  " << it->first << " -> " << it->second << std::endl;
        std::cout << "locations: " << server.locations.size() << std::endl;

        for (size_t j = 0; j < server.locations.size(); ++j)
        {
            const LocationConfig &location = server.locations[j];
            std::cout << "\n  LOCATION " << location.path << std::endl;
            std::cout << "  root: " << location.root << std::endl;
            std::cout << "  index: " << location.index << std::endl;
            std::cout << "  autoindex: " << location.autoindex << std::endl;
            std::cout << "  allowed_methods:";
            for (size_t k = 0; k < location.allowed_methods.size(); ++k)
                std::cout << " " << location.allowed_methods[k];
            std::cout << std::endl;
            std::cout << "  error_pages:" << std::endl;
            for (std::map<int, std::string>::const_iterator it = location.error_pages.begin(); it != location.error_pages.end(); ++it)
                std::cout << "    " << it->first << " -> " << it->second << std::endl;
            std::cout << "  redirect_code: " << location.redirect_code << std::endl;
            std::cout << "  redirect_url: " << location.redirect_url << std::endl;
            std::cout << "  upload_path: " << location.upload_path << std::endl;
            std::cout << "  cgi_handlers:" << std::endl;
            for (std::map<std::string, std::string>::const_iterator it = location.cgi_handlers.begin(); it != location.cgi_handlers.end(); ++it)
                std::cout << "    " << it->first << " -> " << it->second << std::endl;
        }
    }

    std::cout << "\n============================" << std::endl;
}

bool Config::hasValidValue(const std::vector<std::string> &tokens, size_t i, const std::string &directive) const
{
    if (i >= tokens.size() || tokens[i] == ";" || tokens[i] == "{" || tokens[i] == "}")
    {
        std::cerr << "Error: missing value after '" << directive << "'" << std::endl;
        return false;
    }

    return true;
}

std::vector<std::string> Config::tokenize(const std::string &content) const
{
    std::vector<std::string> tokens;
    std::string current; // contiene la parola che stiamo costruendo.

    for (size_t i = 0; i < content.size(); ++i)
    {
        char c = content[i];

        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }
        }
        else if (c == '{' || c == '}' || c == ';')
        {
            if (!current.empty())
            {
                tokens.push_back(current);
                current.clear();
            }

            tokens.push_back(std::string(1, c));
        }
        else
            current += c;
    }
    if (!current.empty())
        tokens.push_back(current);
    return tokens;
}

bool Config::isValidPort(const std::string &value) const
{
    if (value.empty())
        return false;

    for (size_t i = 0; i < value.size(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(value[i])))
            return false;
    }
    long port = std::atol(value.c_str());
    if (port < 1 || port > 65535)
        return false;
    return true;
}

bool Config::isValidIPv4(const std::string &address) const
{
    if (address.empty())
        return false;

    size_t start = 0;
    int parts = 0;

    while (start <= address.size())
    {
        size_t end = address.find('.', start);
        if (end == std::string::npos)
            end = address.size();
        std::string part = address.substr(start, end - start);
        if (part.empty() || part.size() > 3)
            return false;
        for (size_t i = 0; i < part.size(); ++i)
        {
            if (!std::isdigit(static_cast<unsigned char>(part[i])))
                return false;
        }
        long number = std::atol(part.c_str());
        if (number < 0 || number > 255)
            return false;
        ++parts;
        if (end == address.size())
            break;
        start = end + 1;
    }
    return parts == 4;
}

bool Config::parseListen(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server)
{
    if (i >= tokens.size() || tokens[i] != "listen")
    {
        std::cerr << "Error: expected 'listen'" << std::endl;
        return false;
    }
    if (!server.port.empty() || !server.address.empty())
    {
        std::cerr << "Error: duplicate listen directive in server block" << std::endl;
        return false;
    }
    ++i;
    if (i >= tokens.size())
    {
        std::cerr << "Error: missing value after 'listen'" << std::endl;
        return false;
    }
    const std::string value = tokens[i];
    if (!hasValidValue(tokens, i, "listen"))
        return false;
    size_t colon = value.find(':');
    if (colon == std::string::npos)
    {
        if (!isValidPort(value))
        {
            std::cerr << "Error: invalid listen port '" << value << "'" << std::endl;
            return false;
        }
        server.address = "0.0.0.0";
        server.port = value;
    }
    else
    {
        if (value.find(':', colon + 1) != std::string::npos)
        {
            std::cerr << "Error: invalid listen value '" << value << "'" << std::endl;
            return false;
        }
        std::string address = value.substr(0, colon);
        std::string port = value.substr(colon + 1);
        if (!isValidIPv4(address))
        {
            std::cerr << "Error: invalid listen address '" << address << "'" << std::endl;
            return false;
        }
        if (!isValidPort(port))
        {
            std::cerr << "Error: invalid listen port '" << port << "'" << std::endl;
            return false;
        }

        server.address = address;
        server.port = port;
    }
    ++i;
    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after listen directive" << std::endl;
        return false;
    }
    ++i;
    return true;
}

bool Config::parseServerName(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server)
{
    if (i >= tokens.size() || tokens[i] != "server_name")
    {
        std::cerr << "Error: expected 'server_name'" << std::endl;
        return false;
    }
    if (!server.server_name.empty())
    {
        std::cerr << "Error: duplicate server_name directive" << std::endl;
        return false;
    }
    ++i;
    if (!hasValidValue(tokens, i, "server_name"))
        return false;
    server.server_name = tokens[i];
    ++i;
    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after server_name" << std::endl;
        return false;
    }
    ++i;
    return true;
}

bool Config::parseRoot(const std::vector<std::string> &tokens, size_t &i, std::string &root)
{
    if (i >= tokens.size() || tokens[i] != "root")
    {
        std::cerr << "Error: expected 'root'" << std::endl;
        return false;
    }
    if (!root.empty())
    {
        std::cerr << "Error: duplicate root directive" << std::endl;
        return false;
    }
    ++i;
    if (!hasValidValue(tokens, i, "root"))
        return false;

    root = tokens[i];
    ++i;
    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after root" << std::endl;
        return false;
    }
    ++i;
    return true;
}

bool Config::parseIndex(const std::vector<std::string> &tokens, size_t &i, std::string &index)
{
    if (i >= tokens.size() || tokens[i] != "index")
    {
        std::cerr << "Error: expected 'index'" << std::endl;
        return false;
    }
    if (!index.empty())
    {
        std::cerr << "Error: duplicate index directive" << std::endl;
        return false;
    }
    ++i;
    if (!hasValidValue(tokens, i, "index"))
        return false;
    index = tokens[i];
    ++i;
    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after index" << std::endl;
        return false;
    }
    ++i;
    return true;
}
bool Config::parseAutoindex(const std::vector<std::string> &tokens, size_t &i, int &autoindex)
{
    if (i >= tokens.size() || tokens[i] != "autoindex")
    {
        std::cerr << "Error: expected 'autoindex'" << std::endl;
        return false;
    }
    if (autoindex != -1)
    {
        std::cerr << "Error: duplicate autoindex directive" << std::endl;
        return false;
    }
    ++i;
    if (!hasValidValue(tokens, i, "autoindex"))
        return false;
    if (tokens[i] == "on")
        autoindex = 1;
    else if (tokens[i] == "off")
        autoindex = 0;
    else
    {
        std::cerr << "Error: invalid autoindex value '" << tokens[i] << "'" << std::endl;
        return false;
    }
    ++i;
    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after autoindex" << std::endl;
        return false;
    }
    ++i;
    return true;
}

/*
Controlla:
che il token iniziale sia allowed_methods;
che la direttiva non sia già stata dichiarata;
che ci sia almeno un metodo;
che ogni metodo sia tra GET, POST, DELETE;
che lo stesso metodo non compaia due volte;
che la direttiva termini con ;;
che l’indice i venga lasciato correttamente sul token successivo.
*/
bool Config::parseAllowedMethods(const std::vector<std::string> &tokens, size_t &i, std::vector<std::string> &allowed_methods)
{
    if (i >= tokens.size() || (tokens[i] != "allowed_methods" && tokens[i] != "methods"))
    {
        std::cerr << "Error: expected 'allowed_methods'" << std::endl;
        return false;
    }
    if (!allowed_methods.empty())
    {
        std::cerr << "Error: duplicate allowed_methods directive" << std::endl;
        return false;
    }
    ++i;
    if (!hasValidValue(tokens, i, "allowed_methods"))
        return false;

    while (i < tokens.size() && tokens[i] != ";")
    {
        if (tokens[i] != "GET" && tokens[i] != "POST" && tokens[i] != "DELETE")
        {
            std::cerr << "Error: invalid HTTP method '" << tokens[i] << "'" << std::endl;
            return false;
        }

        for (size_t j = 0; j < allowed_methods.size(); ++j)
        {
            if (allowed_methods[j] == tokens[i])
            {
                std::cerr << "Error: duplicate HTTP method '" << tokens[i] << "'" << std::endl;
                return false;
            }
        }
        allowed_methods.push_back(tokens[i]);
        ++i;
    }
    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after allowed_methods" << std::endl;
        return false;
    }
    ++i;
    return true;
}
bool Config::parseClientMaxBodySize(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server)
{
    if (i >= tokens.size() || tokens[i] != "client_max_body_size")
    {
        std::cerr << "Error: expected 'client_max_body_size'" << std::endl;
        return false;
    }

    if (server.client_max_body_size != 0)
    {
        std::cerr << "Error: duplicate client_max_body_size directive" << std::endl;
        return false;
    }
    ++i;
    if (!hasValidValue(tokens, i, "client_max_body_size"))
        return false;
    const std::string value = tokens[i];
    long size;
    if (!string_to_long(value, size))
    {
        std::cerr << "Error: invalid or too large client_max_body_size value '" << value << "'" << std::endl;
        return false;
    }
    if (size <= 0)
    {
        std::cerr << "Error: client_max_body_size must be greater than 0" << std::endl;
        return false;
    }
    server.client_max_body_size = size;
    ++i;
    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after client_max_body_size" << std::endl;
        return false;
    }
    ++i;
    return true;
}

bool Config::parseErrorPage(const std::vector<std::string> &tokens, size_t &i, std::map<int, std::string> &error_pages)
{
    if (i >= tokens.size() || tokens[i] != "error_page")
    {
        std::cerr << "Error: expected 'error_page'" << std::endl;
        return false;
    }
    ++i;
    if (!hasValidValue(tokens, i, "error_page"))
        return false;
    std::vector<int> error_codes;
    /*
     * Servono almeno:
     * - un codice
     * - un path
     * - il punto e virgola
     * Continuiamo a leggere codici finché, dopo il token corrente,
     * non troviamo il path seguito da ';'.
     */
    while (i < tokens.size() &&
           tokens[i] != ";" &&
           i + 1 < tokens.size() &&
           tokens[i + 1] != ";")
    {
        const std::string &code_value = tokens[i];

        for (size_t j = 0; j < code_value.size(); ++j)
        {
            if (!std::isdigit(static_cast<unsigned char>(code_value[j])))
            {
                std::cerr << "Error: invalid error code '" << code_value << "'" << std::endl;
                return false;
            }
        }

        int error_code = std::atoi(code_value.c_str());

        if (error_code < 300 || error_code > 599)
        {
            std::cerr << "Error: error code must be between 300 and 599" << std::endl;
            return false;
        }
        if (error_pages.count(error_code) != 0)
        {
            std::cerr << "Error: duplicate error page for code " << error_code << std::endl;
            return false;
        }
        for (size_t j = 0; j < error_codes.size(); ++j)
        {
            if (error_codes[j] == error_code)
            {
                std::cerr << "Error: duplicate error code " << error_code << std::endl;
                return false;
            }
        }
        error_codes.push_back(error_code);
        ++i;
    }
    if (error_codes.empty())
    {
        std::cerr << "Error: missing error code after 'error_page'" << std::endl;
        return false;
    }

    if (!hasValidValue(tokens, i, "error_page"))
        return false;

    const std::string path = tokens[i];
    ++i;

    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after error_page" << std::endl;
        return false;
    }

    for (size_t j = 0; j < error_codes.size(); ++j)
        error_pages[error_codes[j]] = path;
    ++i;
    return true;
}

bool Config::parseRedirect(const std::vector<std::string> &tokens, size_t &i, LocationConfig &location)
{
    if (i >= tokens.size() || tokens[i] != "return")
    {
        std::cerr << "Error: expected 'return'" << std::endl;
        return false;
    }

    if (location.redirect_code != 0 || !location.redirect_url.empty())
    {
        std::cerr << "Error: duplicate return directive in location" << std::endl;
        return false;
    }

    ++i;

    if (!hasValidValue(tokens, i, "return"))
        return false;

    long code;

    if (!string_to_long(tokens[i], code))
    {
        std::cerr << "Error: invalid redirect status code '" << tokens[i] << "'" << std::endl;
        return false;
    }

    if (code != 301 && code != 302 && code != 303 && code != 307 && code != 308)
    {
        std::cerr << "Error: unsupported redirect status code " << code << std::endl;
        return false;
    }

    location.redirect_code = static_cast<int>(code);
    ++i;

    if (!hasValidValue(tokens, i, "return"))
        return false;

    location.redirect_url = tokens[i];
    ++i;

    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after return directive" << std::endl;
        return false;
    }

    ++i;
    return true;
}

bool Config::parseUploadPath(const std::vector<std::string> &tokens, size_t &i, LocationConfig &location)
{
    if (i >= tokens.size() || (tokens[i] != "upload_path" && tokens[i] != "upload"))
    {
        std::cerr << "Error: expected 'upload_path'" << std::endl;
        return false;
    }

    if (!location.upload_path.empty())
    {
        std::cerr << "Error: duplicate upload_path directive in location" << std::endl;
        return false;
    }
    ++i;
    if (!hasValidValue(tokens, i, "upload_path"))
        return false;

    location.upload_path = tokens[i];
    ++i;

    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after upload_path" << std::endl;
        return false;
    }

    ++i;
    return true;
}

bool Config::parseCgi(const std::vector<std::string> &tokens, size_t &i, LocationConfig &location)
{
    if (i >= tokens.size() || tokens[i] != "cgi")
    {
        std::cerr << "Error: expected 'cgi'" << std::endl;
        return false;
    }
    ++i;
    if (!hasValidValue(tokens, i, "cgi"))
        return false;
    const std::string extension = tokens[i];
    if (extension.size() < 2 || extension[0] != '.')
    {
        std::cerr << "Error: CGI extension must start with '.'" << std::endl;
        return false;
    }
    if (location.cgi_handlers.count(extension) != 0)
    {
        std::cerr << "Error: duplicate CGI handler for extension '" << extension << "'" << std::endl;
        return false;
    }
    ++i;
    if (!hasValidValue(tokens, i, "cgi"))
        return false;
    const std::string executable = tokens[i];
    ++i;
    if (i >= tokens.size() || tokens[i] != ";")
    {
        std::cerr << "Error: expected ';' after cgi directive" << std::endl;
        return false;
    }
    location.cgi_handlers[extension] = executable;
    ++i;
    return true;
}

bool Config::parseLocation(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server)
{
    LocationConfig location;

    if (i >= tokens.size() || tokens[i] != "location")
    {
        std::cerr << "Error: expected 'location'" << std::endl;
        return false;
    }

    ++i;

    if (!hasValidValue(tokens, i, "location"))
        return false;

    location.path = tokens[i];

    if (location.path.empty() || location.path[0] != '/')
    {
        std::cerr << "Error: location path must start with '/'" << std::endl;
        return false;
    }

    for (size_t j = 0; j < server.locations.size(); ++j)
    {
        if (server.locations[j].path == location.path)
        {
            std::cerr << "Error: duplicate location path '" << location.path << "'" << std::endl;
            return false;
        }
    }

    ++i;

    if (i >= tokens.size() || tokens[i] != "{")
    {
        std::cerr << "Error: expected '{' after location path" << std::endl;
        return false;
    }

    ++i;

    while (i < tokens.size() && tokens[i] != "}")
    {
        if (tokens[i] == "root")
        {
            if (!parseRoot(tokens, i, location.root))
                return false;
        }
        else if (tokens[i] == "index")
        {
            if (!parseIndex(tokens, i, location.index))
                return false;
        }
        else if (tokens[i] == "autoindex")
        {
            if (!parseAutoindex(tokens, i, location.autoindex))
                return false;
        }
        else if (tokens[i] == "allowed_methods" || tokens[i] == "methods")
        {
            if (!parseAllowedMethods(tokens, i, location.allowed_methods))
                return false;
        }
        else if (tokens[i] == "error_page")
        {
            if (!parseErrorPage(tokens, i, location.error_pages))
                return false;
        }
        else if (tokens[i] == "return")
        {
            if (!parseRedirect(tokens, i, location))
                return false;
        }
        else if (tokens[i] == "upload_path" || tokens[i] == "upload")
        {
            if (!parseUploadPath(tokens, i, location))
                return false;
        }
        else if (tokens[i] == "cgi")
        {
            if (!parseCgi(tokens, i, location))
                return false;
        }
        else if (tokens[i] == "location")
        {
            std::cerr << "Error: nested location blocks are not allowed" << std::endl;
            return false;
        }
        else
        {
            std::cerr << "Error: unknown location directive '" << tokens[i] << "'" << std::endl;
            return false;
        }
    }

    if (i >= tokens.size())
    {
        std::cerr << "Error: missing '}' at end of location block" << std::endl;
        return false;
    }

    ++i;

    server.locations.push_back(location);

    return true;
}

bool Config::parseServer(const std::vector<std::string> &tokens, size_t &i)
{
    ServerConfig server;

    if (i >= tokens.size() || tokens[i] != "server")
    {
        std::cerr << "Error: expected 'server'" << std::endl;
        return false;
    }

    ++i;

    if (i >= tokens.size() || tokens[i] != "{")
    {
        std::cerr << "Error: expected '{' after 'server'" << std::endl;
        return false;
    }

    ++i;

    while (i < tokens.size() && tokens[i] != "}")
    {
        if (tokens[i] == "listen")
        {
            if (!parseListen(tokens, i, server))
                return false;
        }
        else if (tokens[i] == "server_name")
        {
            if (!parseServerName(tokens, i, server))
                return false;
        }
        else if (tokens[i] == "root")
        {
            if (!parseRoot(tokens, i, server.root))
                return false;
        }
        else if (tokens[i] == "index")
        {
            if (!parseIndex(tokens, i, server.index))
                return false;
        }
        else if (tokens[i] == "autoindex")
        {
            if (!parseAutoindex(tokens, i, server.autoindex))
                return false;
        }
        else if (tokens[i] == "allowed_methods" || tokens[i] == "methods")
        {
            if (!parseAllowedMethods(tokens, i, server.allowed_methods))
                return false;
        }
        else if (tokens[i] == "error_page")
        {
            if (!parseErrorPage(tokens, i, server.error_pages))
                return false;
        }
        else if (tokens[i] == "client_max_body_size")
        {
            if (!parseClientMaxBodySize(tokens, i, server))
                return false;
        }
        else if (tokens[i] == "location")
        {
            if (!parseLocation(tokens, i, server))
                return false;
        }
        else
        {
            std::cerr << "Error: unknown server directive '" << tokens[i] << "'" << std::endl;
            return false;
        }
    }
    if (i >= tokens.size())
    {
        std::cerr << "Error: missing '}' at end of server block" << std::endl;
        return false;
    }

    ++i;

    /* listen è obbligatorio */
    if (server.port.empty())
    {
        std::cerr << "Error: missing listen directive in server block" << std::endl;
        return false;
    }

    /* valori di default */
    if (server.root.empty())
        server.root = "./www";

    if (server.index.empty())
        server.index = "index.html";

    if (server.autoindex == -1)
        server.autoindex = 0;

    if (server.allowed_methods.empty())
        server.allowed_methods.push_back("GET");

    if (server.client_max_body_size == 0)
        server.client_max_body_size = 1000000;

    /* controllo duplicati address:port */
    for (size_t j = 0; j < configs.size(); ++j)
    {
        if (configs[j].address == server.address && configs[j].port == server.port)
        {
            std::cerr << "Error: duplicate listen address:port '" << server.address << ":" << server.port << "'" << std::endl;
            return false;
        }
    }

    /* solo ora salvo il server */
    configs.push_back(server);

    return true;
}

bool Config::validateConfig() const
{
    if (configs.empty())
    {
        std::cerr << "Error: configuration must contain at least one server block" << std::endl;
        return false;
    }

    for (size_t i = 0; i < configs.size(); ++i)
    {
        const ServerConfig &server = configs[i];

        if (server.address.empty() || server.port.empty())
        {
            std::cerr << "Error: server " << i + 1 << " has no valid listen configuration" << std::endl;
            return false;
        }

        for (size_t j = 0; j < server.locations.size(); ++j)
        {
            const LocationConfig &location = server.locations[j];
            if (location.path.empty() || location.path[0] != '/')
            {
                std::cerr << "Error: invalid location path in server " << i + 1 << std::endl;
                return false;
            }
            if (!location.upload_path.empty() &&
                location.allowed_methods.empty())
            {
                std::cerr << "Error: upload location '" << location.path << "' must define allowed methods" << std::endl;
                return false;
            }
            if (!location.upload_path.empty())
            {
                bool post_allowed = false;

                for (size_t k = 0; k < location.allowed_methods.size(); ++k)
                {
                    if (location.allowed_methods[k] == "POST")
                    {
                        post_allowed = true;
                        break;
                    }
                }

                if (!post_allowed)
                {
                    std::cerr << "Error: upload location '" << location.path << "' must allow POST" << std::endl;
                    return false;
                }
            }
            if (location.redirect_code != 0 &&
                location.redirect_url.empty())
            {
                std::cerr << "Error: redirect location '" << location.path << "' has no destination" << std::endl;
                return false;
            }
        }
    }

    return true;
}

bool Config::parse_config(const std::string &conf_file)
{
    configs.clear();
    int conf_fd;
    char buffer[4096];
    ssize_t bytes_read;
    std::string content;

    conf_fd = open(conf_file.c_str(), O_RDONLY);
    if (conf_fd < 0)
    {
        std::cerr << "Error: couldn't open config file" << std::endl;
        return false;
    }
    while ((bytes_read = read(conf_fd, buffer, sizeof(buffer))) > 0)
        content.append(buffer, bytes_read);
    if (bytes_read < 0)
    {
        std::cerr << "Error: couldn't read config file" << std::endl;
        close(conf_fd);
        return false;
    }
    close(conf_fd);
    std::vector<std::string> tokens = tokenize(content);
    size_t i = 0;

    while (i < tokens.size())
    {
        if (tokens[i] == "server")
        {
            if (!parseServer(tokens, i))
                return false;
        }
        else
        {
            std::cerr << "Error: unexpected token '" << tokens[i] << "'" << std::endl;
            return false;
        }
    }
    if (!validateConfig())
        return false;

    return true;
}
