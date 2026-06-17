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

    bool has_full_header(const char *data, size_t len);
    const std::string &get_request() const;
    bool has_full_headers() const;
    void clear_request();
    bool prepare_response(std::string response);

  private:
    int client_fd;
    std::string request_buffer;
    std::string method;
    std::string path;
    std::string version;
    std::string response_buffer;
    size_t _bytes_sent;
};
