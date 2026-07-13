#include "Server.hpp"
#include "webserv.hpp"
#include "Config.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Error: usage: " << argv[0] << " conf_file" << std::endl;
        return 1;
    }
    Server server;
    Config conf;
    //testing parsing config
    conf.parse(argv[1]);

    if (!server.setup(argv[1]))
        return 1;
    if (!server.run())
        return 1;
}
