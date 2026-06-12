#pragma once

#include <fcntl.h>
#include <unistd.h>

class Utils {
public:
    static bool set_nonblocking(int fd);
};
