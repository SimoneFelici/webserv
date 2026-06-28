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


bool read_file(const std::string& file_path, std::string& body)
{
    struct stat file_stat; 
    /* una struttura già esistente, definita dalla libreria <sys/stat.h>
    Questa struttura serve a contenere informazioni su un file.
    file_stat.st_mode contiene informazioni sul “tipo” del path: file normale, directory, link simbolico ecc.
*/
    if (stat(file_path.c_str(), &file_stat) == -1) // stat(percorso_del_file, indirizzo_della_struct_dove_scrivere)
        return false;

    if (!S_ISREG(file_stat.st_mode)) //Questa è una macro. e st_mode contiene sia informazioni sui permessi, sia informazioni sul tipo di file.
        return false;

    int fd = open(file_path.c_str(), O_RDONLY);
    if (fd == -1)
        return false;

    body.clear();

    char buffer[4096];
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0)
    {
        body.append(buffer, bytes_read);
    }

    close(fd);

    if (bytes_read == -1)
        return false;

    return true;
}
