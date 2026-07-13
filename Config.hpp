#pragma once

#include <string>
#include <vector>
#include <map>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

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
    std::string version;
    std::string port;
    std::string root;
    std::string index;
    bool autoindex;
    int max_conn;
    std::map<int, std::string> error_pages;
    std::vector<std::string> allowed_methods;
    std::vector<LocationConfig> locations;

    ServerConfig();
};

class Config {
    private:
    std::vector<ServerConfig> configs;

    std::vector<std::string> tokenize(const std::string &content) const;

    public:
    Config();
    ~Config();

    bool parse(const std::string &file_path);
    const std::vector<ServerConfig> &getConfigs() const;

};

