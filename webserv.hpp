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

#define FILE_BUFFER_SIZE 4096
#define MAX_HEADER_SIZE (8 * 1024)

bool set_nonblocking(int fd);
int read_file(const std::string &file_path, std::string &body);
void to_lower(std::string &s);
std::string trim(const std::string &s);
std::string get_content_type(const std::string &file_path);
bool string_to_long(const std::string &value, long &result);
