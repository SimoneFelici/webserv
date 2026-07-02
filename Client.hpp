#pragma once

#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct ServerConfig;
struct LocationConfig;

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
    bool req_error() const;
    bool prepare_error_response(int error_code);

    // Getters
    const std::string &get_method() const;
    const std::string &get_path() const;
    const std::string &get_version() const;
    const std::string &get_body() const;
    std::string get_header(const std::string &key) const;

    // Response
    bool clear_response();
    bool prepare_response(ServerConfig &config);
    const std::string &get_response() const;
    std::size_t get_bytes_sent() const;
    void add_bytes_sent(std::size_t bytes);

    // Methods
    void build_error_response(int error_code);
    bool handle_get_req(ServerConfig &config, const LocationConfig *loc);

  private:
    struct HttpRequest
    {
        enum State
        {
            PARSING_REQUEST_LINE,
            PARSING_HEADERS,
            PARSING_BODY,
            DONE,
            ERROR
        };

        State state;

        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
        std::size_t body_start;

        HttpRequest() : state(PARSING_REQUEST_LINE), body_start(0) {}
    };

    struct HttpResponse
    {
        std::string version;
        std::string content_type;
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
    HttpResponse res;

    bool parse_request_line(std::size_t &pos);
    bool parse_headers(std::size_t &pos);
    bool parse_body(std::size_t &pos);
    void build_response_buffer();

    const LocationConfig *match_location(const ServerConfig &config) const;
    bool is_method_allowed(const std::vector<std::string> &allowed) const;
    int validate_req(ServerConfig &config, const LocationConfig *&loc);
};
