#include "Config.hpp"
#include "Server.hpp"

LocationConfig::LocationConfig() : autoindex(false)
{
}

ServerConfig::ServerConfig() : autoindex(false), max_conn(-1), client_max_body_size(-1)
{
}

Config::Config() {}

Config::~Config() {}

const std::vector<ServerConfig> &Config::getConfigs() const
{
    return configs;
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
        {
            current += c;
        }
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

    ++i;

    if (i >= tokens.size())
    {
        std::cerr << "Error: missing value after 'listen'" << std::endl;
        return false;
    }

    const std::string value = tokens[i];

    if (value == ";" || value == "{" || value == "}")
    {
        std::cerr << "Error: invalid listen value" << std::endl;
        return false;
    }

    size_t colon = value.find(':');

    if (colon == std::string::npos)
    {
        if (!isValidPort(value))
        {
            std::cerr << "Error: invalid listen port '"
                      << value << "'" << std::endl;
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
        // else if (tokens[i] == "root")
        // {
        //     // parseRoot()
        // }
        // else if (tokens[i] == "index")
        // {
        //     // parseIndex()
        // }
        // else if (tokens[i] == "autoindex")
        // {
        //     // parseAutoindex()
        // }
        // else if (tokens[i] == "allowed_methods")
        // {
        //     // parseAllowedMethods()
        // }
        // else if (tokens[i] == "error_page")
        // {
        //     // parseErrorPage()
        // }
        // else if (tokens[i] == "client_max_body_size")
        // {
        //     // parseClientMaxBodySize()
        // }
        // else if (tokens[i] == "location")
        // {
        //     // parseLocation()
        // }
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
    configs.push_back(server);

    return true;
}

bool Config::parse_config(const std::string &conf_file)
{
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

    return true;
}