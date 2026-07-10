#pragma once

#include <cctype>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// HARDCODED PARSED CONFIG
#define PORT "8081"
// #define ADDRESS "127.0.0.1"
#define ADDRESS "0.0.0.0" // per docker
#define MAX_CONN SOMAXCONN

bool set_nonblocking(int fd);
int read_file(const std::string &file_path, std::string &body);
void to_lower(std::string &s);
std::string trim(const std::string &s);
std::string get_content_type(const std::string &file_path);
