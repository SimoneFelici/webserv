#include "Server.hpp"
#include "webserv.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Error: usage: " << argv[0] << " conf_file" << std::endl;
        return 1;
    }
    Server server;

    if (!server.start(argv[1]))
    {
        // RETURNS AND CALLS THE DECONSTRUCTOR THAT CLEANS UP
        return 1;
    }
}
