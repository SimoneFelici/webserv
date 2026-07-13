#include "Config.hpp"
#include "Server.hpp"


LocationConfig::LocationConfig() : autoindex(false)
{
}

ServerConfig::ServerConfig() : address("0.0.0.0"), version("HTTP/1.1"), autoindex(false), max_conn(128)
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
    std::string current;  // contiene la parola che stiamo costruendo.

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

bool Config::parse(const std::string &conf_file)
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

    if (DEBUG)
    {
        // std::cout << content << std::endl;
        std::vector<std::string> tokens = tokenize(content);
    
        for (size_t i = 0; i < tokens.size(); ++i)
        std::cout << "[" << tokens[i] << "]" << std::endl;

    }

    return true;
}