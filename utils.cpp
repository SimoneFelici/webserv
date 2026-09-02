#include "webserv.hpp"
#include <climits>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

bool set_nonblocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        return false;

    return true;
}

void to_lower(std::string &s)
{
    for (size_t i = 0; i < s.size(); ++i)
        s[i] = static_cast<char>(tolower(static_cast<unsigned char>(s[i])));
}

// toglie spazi e tab in testa e in coda
std::string trim(const std::string &s)
{
    size_t start = 0;
    size_t end = s.size();
    while (start < end && (s[start] == ' ' || s[start] == '\t'))
        ++start;
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t'))
        --end;
    return s.substr(start, end - start);
}

std::string get_content_type(const std::string &file_path)
{
    static std::map<std::string, std::string> mime;
    if (mime.empty())
    {
        mime["html"] = "text/html";
        mime["css"] = "text/css";
        mime["js"] = "application/javascript";
        mime["png"] = "image/png";
        mime["jpg"] = "image/jpeg";
        mime["jpeg"] = "image/jpeg";
        mime["gif"] = "image/gif";
        mime["svg"] = "image/svg+xml";
        mime["ico"] = "image/x-icon";
        mime["txt"] = "text/plain";
        mime["pdf"] = "application/pdf";
        mime["json"] = "application/json";
    }

    size_t dot = file_path.rfind('.');
    std::string::size_type slash = file_path.find_last_of('/');

    if (dot == std::string::npos || dot + 1 == file_path.size())
        return "application/octet-stream";
    if (slash != std::string::npos && (dot < slash || dot == slash + 1))
        return "application/octet-stream";

    std::string ext = file_path.substr(dot + 1);
    to_lower(ext);

    std::map<std::string, std::string>::const_iterator it = mime.find(ext);
    if (it == mime.end())
        return ("application/octet-stream");

    return it->second;
}

int read_file(const std::string &file_path, std::string &body)
{
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1)
    {
        if (errno == ENOENT || errno == ENOTDIR)
            return 404;
        if (errno == EACCES)
            return 403;
        if (errno == EISDIR)
            return 403;
        return 500;
    }

    body.clear();

    char buffer[FILE_BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
        body.append(buffer, bytes_read);

    if (bytes_read == -1)
    {
        close(fd);
        return 500;
    }

    close(fd);
    return 200;
}

bool string_to_long(const std::string &value, long &result)
{
    result = 0;

    if (value.empty())
        return false;

    for (size_t i = 0; i < value.size(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(value[i])))
            return false;

        int digit = value[i] - '0';

        if (result > (LONG_MAX - digit) / 10)
            return false;

        result = result * 10 + digit;
    }

    return true;
}
