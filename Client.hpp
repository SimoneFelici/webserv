#pragma once

class Client {
public:
    Client();
    ~Client();

private:
    Client(const Client& other);
    Client& operator=(const Client& other);

    int fd;
};
