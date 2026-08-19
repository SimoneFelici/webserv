#pragma once

#include <csignal>
#include <cstdlib>
#include <dirent.h> // ??
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

struct ServerConfig;
struct LocationConfig;

struct MultipartPart
{
    std::string name;
    std::string filename;
    std::string content_type;
    std::string data;
};

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
    bool is_headers_too_large() const;
    const std::string &get_query_string() const;

    // Response
    bool clear_response();
    bool prepare_response(ServerConfig &config);
    const std::string &get_response() const;
    std::size_t get_bytes_sent() const;
    void add_bytes_sent(std::size_t bytes);

    // Methods
    bool handle_get_req(ServerConfig &config, const LocationConfig *loc);
    bool handle_post_req(ServerConfig &config, const LocationConfig *loc);
    bool handle_delete_req(ServerConfig &config, const LocationConfig *loc);

    // CGI
    bool parse_cgi_output(const std::string &output);
    bool exec_cgi(const std::string &script_path, const std::string &script_name, const std::string &interpreter);
    int get_cgi_fd() const;
    pid_t get_cgi_pid() const;
    void clear_cgi();
    bool finish_cgi(const std::string &output, ServerConfig &config);

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
        std::string query_string;

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

    bool headers_too_large;

    int cgi_fd;
    pid_t cgi_pid;

    std::string get_error_reason(int error_code) const;

    bool parse_request_line(std::size_t &pos);
    bool parse_header_line(const std::string &line);
    bool parse_headers(std::size_t &pos);
    bool parse_body(std::size_t &pos);
    void build_error_response(int error_code, const ServerConfig &config, const LocationConfig *loc);
    void build_default_error_response(int error_code);
    void build_response_buffer();
    void build_redirect_response(const LocationConfig &loc);

    const LocationConfig *match_location(const ServerConfig &config) const;
    bool is_method_allowed(const std::vector<std::string> &allowed) const;
    int sanitize_path();
    int validate_req(ServerConfig &config, const LocationConfig *&loc);
    std::string build_file_path(const ServerConfig &config, const LocationConfig *loc) const;

    // POST funzioni multipart
    int write_uploaded_file(const std::string &file_path, const std::string &data);
    bool extract_multipart_boundary(const std::string &content_type, std::string &boundary) const;
    int parse_multipart_body(const std::string &body, const std::string &boundary, std::vector<MultipartPart> &parts) const;
    bool parse_multipart_part_headers(const std::string &headers_block, MultipartPart &part) const;
    int handle_raw_upload(const LocationConfig *loc);
    int handle_multipart_upload( const LocationConfig *loc, const std::string &content_type);


    bool split_cgi_path(const LocationConfig *loc, std::string &script_name, std::string &interpreter) const;
};
