#pragma once

#include <string>

class Client
{
  public:
    Client(int fd);
    Client();
    ~Client();

    int get_fd() const;
    void set_fd(int fd);

    void append_request(const char *data, size_t len);
    const std::string &get_request() const;

  private:
    int fd;
    std::string request_buffer;
    std::string response_buffer;
    size_t _bytes_sent;
};
