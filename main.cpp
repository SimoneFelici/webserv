#include "Client.hpp"
#include "Server.hpp"

int main()
{
    Server server;

    if (!server.start())
        return 1;
}
