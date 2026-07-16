#pragma once

#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h> // ?? 
#include <sys/stat.h>

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
    void print_response() const;
    // TODO: USE AFTER SENDING THE RESPONSE
    void clear_request();

    bool parse_request();
    bool req_done() const;
    bool req_error() const;
    bool prepare_error_response(int error_code, const ServerConfig &config);

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
    
    bool is_allowed_method(ServerConfig &config);
    
    // Methods
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

        HttpResponse() : status_code(200) {}
    };

    int client_fd;

    std::string request_buffer;
    std::string response_buffer;

    std::size_t bytes_sent;

    HttpRequest req;
    HttpResponse res;

    std::string get_error_reason(int error_code) const;

    bool parse_request_line(std::size_t &pos);
    bool parse_header_line(const std::string &line);
    bool parse_headers(std::size_t &pos);
    bool parse_body(std::size_t &pos);
    void build_error_response(int error_code, const ServerConfig &config, const LocationConfig *loc);
    void build_default_error_response(int error_code);
    void build_response_buffer();


    const LocationConfig *match_location(const ServerConfig &config) const;
    bool is_method_allowed(const std::vector<std::string> &allowed) const;
    int sanitize_path();
    int validate_req(ServerConfig &config, const LocationConfig *&loc);
    std::string build_file_path(const ServerConfig &config, const LocationConfig *loc) const;
};
