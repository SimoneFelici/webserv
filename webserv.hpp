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
#define ADDRESS "127.0.0.1"
#define MAX_CONN SOMAXCONN

bool set_nonblocking(int fd);
