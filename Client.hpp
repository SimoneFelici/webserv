#pragma once

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>

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

    // Getters
    const std::string &get_method() const;
    const std::string &get_path() const;
    const std::string &get_version() const;
    const std::string &get_body() const;
    std::string get_header(const std::string &key) const;

    //Response
    bool clear_response();
    bool prepare_response(ServerConfig& config); 
    const std::string &get_response() const;
    std::size_t get_bytes_sent() const;
    void add_bytes_sent(std::size_t bytes);

    bool is_allowed_method(ServerConfig &config);

    // Methods
    void build_error_response(int error_code);
    bool handle_get_req(ServerConfig &config);

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
    bool is_method_allowed();
};
