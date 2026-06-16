# webserv

## epoll()

In epoll ci sono due strutture diverse da non confondere:
1. Lista interna di epoll

crei un oggetto nel kernel.
```c++
this->epoll_fd = epoll_create1(0);
```

Poi aggiungi fd da monitorare con:
```c++
epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &event);
```

Esempio:
```c++
epoll_event event;
event.events = EPOLLIN;
event.data.fd = this->fd;

epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->fd, &event);
```

Qui stai dicendo:
- epoll, tieni sotto controllo this->fd
- mi interessa EPOLLIN
- quando succede qualcosa, ricordati che questo evento riguarda this->fd  
Questa informazione resta dentro il kernel, associata a **epoll_fd**.

2. Array **events**

Questo array NON è la lista degli fd monitorati.
È solo un contenitore temporaneo dove epoll_wait() scrive gli eventi pronti.

dentro run()
```c++
const int max_events = 1024;
epoll_event events[max_events];

// poi fai 
int ready = epoll_wait(this->epoll_fd, events, max_events, -1);
// Stai passando EVENTS QUI

```
**epoll_wait() guarda la lista interna di epoll e scrive dentro events solo quelli pronti.**

# epoll_wait()

```c++
ready = epoll_wait(epoll_fd, events, max_events, -1);
```

kernel guarda gli fd registrati
trova quelli pronti
li copia dentro events[]
ritorna quanti ne ha scritti 

**Punto fondamentale**
Questo:
```c++
epoll_event event;

//[...]
epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->fd, &event);
```
usato con **epoll_ctl**, serve per registrare cosa vuoi controllare.
Questo:
```c++
epoll_event events[max_events];
int ready = epoll_wait(this->epoll_fd, events, max_events, -1);
```
usato con **epoll_wait**, serve per ricevere cosa è successo davvero.

**Frase chiave**
*epoll_ctl()* mette fd e interessi dentro epoll.
*epoll_wait()* prende gli eventi pronti da epoll e li scrive nell’array events.

---
# Leggere dal socket

handle_client_read() deve chiamare recv() perché epoll ti ha detto che quel fd è pronto.

```c++
ssize_t recv(int socket_fd, void *buffer, size_t length, int flags);

// Esempio mentale:
char buffer[4096];

ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

```
I parametri sono:
 - client_fd: il file descriptor del client da cui vuoi leggere
- buffer: l'array temporaneo dove recv metterà i dati ricevuti
- sizeof(buffer): quanti byte massimo sei disposta a ricevere in questa chiamata
- 0: flags

**Il valore di ritorno è fondamentale:**
* bytes_read > 0 : hai ricevuto bytes_read byte
* bytes_read == 0 : il client ha chiuso la connessione
* bytes_read < 0 : errore


Quindi:
se ricevi byte, li aggiungi al buffer del client;
se ricevi 0, il client se n’è andato: va chiuso;
se ricevi errore, per ora chiudi il client.

### Ma la richiesta HTTP arriva tutta insieme?
Può arrivare tutta insieme, ma non devi mai darlo per scontato. Come si gestisce se arriva a pezzi?

### Come capiamo se la richiesta HTTP è completa?










---
# MODIFICHE DI OGGI:
- sistemato il distruttore del Server aggiungendo chiusura fd epoll e sistemata anche run()
- aggiunte alla classe client

- logica di handle_client_read(client_fd)
```

1. cerca client_fd dentro clients
   se non esiste:
       return false

2. crea un buffer temporaneo, tipo char temp[4096]

3. chiama recv(client_fd, temp, sizeof(temp), 0)

4. se recv ritorna 0:
       return false

5. se recv ritorna < 0:
       return false

6. se recv ritorna > 0:
       aggiungi quei byte al request_buffer del Client -> da implementare nel client

7. controlla il request_buffer:  -> implementerei anche questo a seconda della risposta del client
       contiene "\r\n\r\n"?
           no:
               return true

           sì:
               prepara response provvisoria nel Client
               modifica epoll: da EPOLLIN a EPOLLOUT
               return true
```

