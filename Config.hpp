#pragma once

#include <string>
#include <vector>
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include "webserv.hpp"

struct LocationConfig
{
    std::string path;
    std::string root;
    std::string index;
    bool autoindex;
    std::vector<std::string> allowed_methods;
    std::map<int, std::string> error_pages;

    LocationConfig();
};

struct ServerConfig
{
    std::string address;
    std::string port;
    std::string server_name;
    std::string version;
    std::string root;
    std::string index;
    bool autoindex;
    int max_conn;
    size_t client_max_body_size;
    std::map<int, std::string> error_pages;
    std::vector<std::string> allowed_methods;
    std::vector<LocationConfig> locations;

    ServerConfig();
};

class Config {
    private:
    std::vector<ServerConfig> configs;

    bool isValidPort(const std::string &value) const;
    bool isValidIPv4(const std::string &address) const;
    bool hasValidValue(const std::vector<std::string> &tokens,  size_t i, const std::string &directive) const;
    
    std::vector<std::string> tokenize(const std::string &content) const;

    bool parseServer(const std::vector<std::string> &tokens, size_t &i);
    bool parseListen(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server);
    bool parseServerName(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server);
    bool parseRoot(const std::vector<std::string> &tokens,
               size_t &i,
               std::string &root);
    bool parseIndex(const std::vector<std::string> &tokens,
                size_t &i,
                std::string &index);
    bool parseAutoindex(const std::vector<std::string> &tokens,
                    size_t &i,
                    bool &autoindex);
    bool parseAllowedMethods(
    const std::vector<std::string> &tokens,
    size_t &i,
    std::vector<std::string> &allowed_methods);
    bool parseClientMaxBodySize(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server);
    bool parseErrorPage(
    const std::vector<std::string> &tokens,
    size_t &i,
    std::map<int, std::string> &error_pages);

    public:
    Config();
    ~Config();

    bool parse_config(const std::string &file_path);
    const std::vector<ServerConfig> &getConfigs() const;

};
