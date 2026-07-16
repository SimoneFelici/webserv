#pragma once

#include "webserv.hpp"
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

struct LocationConfig
{
    std::string path;
    std::string root;
    std::string index;
    int autoindex;
    std::vector<std::string> allowed_methods;
    std::map<int, std::string> error_pages;
    int redirect_code;
    std::string redirect_url;
    std::string upload_path;
    std::map<std::string, std::string> cgi_handlers;

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
    int autoindex;
    int max_conn;
    size_t client_max_body_size;
    std::map<int, std::string> error_pages;
    std::vector<std::string> allowed_methods;
    std::vector<LocationConfig> locations;

    ServerConfig();
};

class Config
{
  private:
    std::vector<ServerConfig> configs;

    bool isValidPort(const std::string &value) const;
    bool isValidIPv4(const std::string &address) const;
    bool hasValidValue(const std::vector<std::string> &tokens, size_t i, const std::string &directive) const;

    std::vector<std::string> tokenize(const std::string &content) const;

    bool parseServer(const std::vector<std::string> &tokens, size_t &i);
    bool parseListen(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server);
    bool parseServerName(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server);
    bool parseRoot(const std::vector<std::string> &tokens, size_t &i, std::string &root);
    bool parseIndex(const std::vector<std::string> &tokens, size_t &i, std::string &index);
    bool parseAllowedMethods(const std::vector<std::string> &tokens, size_t &i, std::vector<std::string> &allowed_methods);
    bool parseClientMaxBodySize(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server);
    bool parseErrorPage(const std::vector<std::string> &tokens, size_t &i, std::map<int, std::string> &error_pages);
    bool parseRedirect(const std::vector<std::string> &tokens, size_t &i, LocationConfig &location);
    bool parseUploadPath(const std::vector<std::string> &tokens, size_t &i, LocationConfig &location);
    bool parseCgi(const std::vector<std::string> &tokens, size_t &i, LocationConfig &location);
    bool parseLocation(const std::vector<std::string> &tokens, size_t &i, ServerConfig &server);
    bool parseAutoindex(const std::vector<std::string> &tokens, size_t &i, int &autoindex);
    bool validateConfig() const;


  public:
    Config();
    ~Config();

    bool parse_config(const std::string &file_path);
    const std::vector<ServerConfig> &getConfigs() const;
    void printConfig() const;
};
