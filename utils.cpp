#include "webserv.hpp"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

bool set_nonblocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        return false;

    return true;
}

// da inserire check su i metodi consentiti per route e su i status code di ritorno 
/*
read_file() non deve tornare solo bool
deve distinguere:
file non esiste       -> 404
path è directory      -> dipende: index / autoindex / 403
non ho permessi       -> 403
open/read fallisce    -> 500 oppure 403, dipende
*/
int read_file(const std::string& file_path, std::string& body)
{
    struct stat file_stat; 
    
    if (stat(file_path.c_str(), &file_stat) == -1) // stat(percorso_del_file, indirizzo_della_struct_dove_scrivere)
        return (404);

    if (!S_ISREG(file_stat.st_mode)) //Questa è una macro. e st_mode contiene informazioni
        return (403);
    
    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1)
        return (500);

    body.clear();

    char buffer[4096];
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        body.append(buffer, bytes_read);
    }

    close(fd);

    if (bytes_read == -1)
        return (500);

    return (200);
}
