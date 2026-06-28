#pragma once

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// HARDCODED PARSED CONFIG
#define PORT "8080"
//#define ADDRESS "127.0.0.1"
#define ADDRESS "0.0.0.0" // per docker
#define MAX_CONN SOMAXCONN

bool set_nonblocking(int fd);
bool read_file(const std::string& file_path, std::string& body);
