#pragma once

class Client {
public:
    Client(int fd);
    Client();
    Client(const Client& other);
    ~Client();

    Client& operator=(const Client& other);

    int get_fd() const;
    void set_fd(int fd);

private:
    int fd;
};
