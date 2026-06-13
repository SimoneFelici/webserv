#include "Client.hpp"
#include "Server.hpp"

int main()
{
    Server server;

    if (!server.start()) {
        // RETURNS AND CALLS THE DECONSTRUCTOR THAT CLEANS UP
        return 1;
    }
}
