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

# HTTP request 

```
 Client 5 read 704 bytes
Request: GET / HTTP/1.1
Host: localhost:8080
Connection: keep-alive
Cache-Control: max-age=0
sec-ch-ua: "Google Chrome";v="147", "Not.A/Brand";v="8", "Chromium";v="147"
sec-ch-ua-mobile: ?0
sec-ch-ua-platform: "Linux"
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/147.0.0.0 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
Sec-Fetch-Site: cross-site
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
Accept-Encoding: gzip, deflate, br, zstd
Accept-Language: it-IT,it;q=0.9,en-US;q=0.8,en;q=0.7
```

### 1. Request line
 
Coposta da:
```
GET / HTTP/1.1
METHOD PATH VERSION
```
è la request line.

Vuol dire:
```
metodo:  GET
path:    /
version: HTTP/1.1
```
Tutte le righe dopo sono headers.

### 2. Headers

Dopo la request line arrivano gli headers.

Gli headers sono righe fatt così:
```
Chiave: valore
```
Esempio:
```
Host: localhost:8080
```

I più importanti della tua request:

**Host: localhost:8080**

Dice al server quale host il browser sta cercando di raggiungere.
In HTTP/1.1 l’header Host è importante, perché uno stesso server può teoricamente servire più siti o più configurazioni.

---

**Connection: keep-alive**  
Il browser ti sta dicendo: 
dopo la response, se puoi, non chiudere subito la connessione. Tienila aperta, così posso riusarla.

---

**Cache-Control: max-age=0**

Il browser sta dicendo qualcosa tipo:

voglio una risposta fresca, non usare una copia vecchia dalla cache.

---

**sec-ch-ua, sec-ch-ua-mobile, sec-ch-ua-platform**  
```
sec-ch-ua: "Google Chrome";v="147", 
"Not.A/Brand";v="8", 
"Chromium";v="147"
sec-ch-ua-mobile: ?0
sec-ch-ua-platform: "Linux"
```
Sono headers moderni del browser. Danno informazioni sul browser/piattaforma.

Nel tuo caso dicono più o meno:

- browser: Chrome/Chromium
- mobile: no
- sistema: Linux

---

**Upgrade-Insecure-Requests: 1**

Il browser dice:

se esiste una versione HTTPS della risorsa, preferirei quella.

Tu stai facendo un server HTTP normale, quindi per ora ignori.

---
**User-Agent**  

User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 ...

Questo descrive il browser/client.

---

**Accept**
```
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,...
```
Qui il browser dice:

questi sono i tipi di contenuto che so ricevere.

Per esempio:
```
text/html                  pagine HTML
application/xhtml+xml      XHTML
application/xml            XML
image/avif                 immagini AVIF
image/webp                 immagini WebP
image/apng                 immagini APNG
*/*                        qualsiasi cosa
```
Più avanti, quando servirai file veri, tu dovrai rispondere con un Content-Type adatto.

Esempio:

Content-Type: text/html

---

**Sec-Fetch-***
```
Sec-Fetch-Site: cross-site
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
```
Sono headers di sicurezza/contesto del browser.

Dicono che tipo di navigazione sta facendo il browser.

Nel tuo caso:

Mode: navigate
Dest: document

Cioè il browser sta navigando verso una pagina/documento.

Per Webserv base li ignori.

---

**Accept-Encoding**

Accept-Encoding: gzip, deflate, br, zstd

Il browser dice:

se vuoi, puoi mandarmi contenuto compresso con gzip, br, zstd, ecc.

---

**Accept-Language**

Accept-Language: it-IT,it;q=0.9,en-US;q=0.8,en;q=0.7

Il browser dice:

preferisco italiano, poi inglese americano, poi inglese.

### 3. Riga vuota finale

Alla fine vedi una riga vuota.

In realtà, internamente, non è “magia”: sono questi caratteri:
```
\r\n\r\n
```
Questa sequenza significa: fine degli headers
```
request_buffer.find("\r\n\r\n")
```
Quindi quando trovi \r\n\r\n, sai che almeno questa parte è completa:

request line + headers