#pragma once

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

class Client
{
  public:
    Client(int fd);
    Client();
    ~Client();

    int get_fd() const;
    void set_fd(int fd);

    bool has_full_headers(const char *data, size_t len);

    const std::string &get_request() const;
    void print_request() const;
    // TODO: USE AFTER SENDING THE RESPONSE
    void clear_request();

    bool parse_request();
    bool req_done() const;

    // Getters
    const std::string &get_path() const;
    const std::string &get_version() const;
    const std::string &get_body() const;
    std::string get_header(const std::string &key) const;

  private:
    struct HttpRequest
    {
        enum State
        {
            PARSING_REQUEST_LINE,
            PARSING_HEADERS,
            PARSING_BODY,
            DONE
        };

        State state;

        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;

        HttpRequest() : state(PARSING_REQUEST_LINE) {}
    };

    struct HttpResponse
    {
        std::string version;
        int status_code;
        std::string reason;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    int client_fd;

    std::string request_buffer;
    std::string response_buffer;

    std::size_t bytes_sent;

    HttpRequest req;

    bool parse_request_line(std::size_t &pos);
    bool parse_headers(std::size_t &pos);
    bool parse_body(std::size_t &pos);
};
