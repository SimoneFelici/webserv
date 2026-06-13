#pragma once

#include "Client.hpp"
#include "Server.hpp"
#include <fcntl.h>
#include <unistd.h>

// HARDCODED PARSED CONFIG
#define PORT 8080
#define ADDRESS INADDR_ANY
#define MAX_CONN SOMAXCONN

bool set_nonblocking(int fd);
