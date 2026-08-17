# webserv

**TODO — Pulizia config e valori hardcodati**

/*FATTO
 Rimuovere PORT, ADDRESS e MAX_CONN hardcodati.
 Usare config.address e config.port nel socket.
 Rimuovere version da ServerConfig.
 Rimuovere max_conn dalla config e usare SOMAXCONN.
 Correggere il main e chiamare server.run().
 Sostituire 4096 e 1024 con macro tecniche nominate.
 Applicare client_max_body_size.
 Usare le error_page personalizzate del server.
 Usare le error_page personalizzate della location.
 Lasciare la pagina errore HTML interna come fallback.
 Applicare i redirect configurati con return.

 DA FARE:
 Controllare eventuali altri valori parsati ma mai utilizzati.
 Ripulire TODO vecchi e codice commentato non più utile.


**TODO — Configurazione**
FATTO!
 Parsing di più blocchi server.
 Parsing di listen.
 Parsing di server_name.
 Parsing di root.
 Parsing di index.
 Parsing di autoindex.
 Parsing di allowed_methods.
 Parsing di client_max_body_size.
 Parsing di error_page.
 Parsing delle location.
 Parsing dei redirect.
 Parsing di upload_path.
 Parsing dei CGI handler.

 DA FARE:
 Decidere se accettare alias come methods, upload oppure usare un solo formato.
 Aggiungere test con configurazioni valide e invalide.
 Verificare meglio i default e centralizzarli con costanti nominate.
 Verificare cartelle e file indicati dalla config, quando necessario.
 Gestire correttamente server con stesso address:port solo se si vogliono virtual host; altrimenti rifiutarli.



**TODO — Multi-server ed epoll**
FATTO:
 Smettere di usare solo configs[0].
 Creare un listening socket per ogni ServerConfig.
 Usare un solo epoll_fd per tutti i server e tutti i client.
 Registrare tutti i listening socket nello stesso epoll.
 Distinguere listening socket e client socket.
 Associare ogni listening socket alla sua ServerConfig.
 Quando arriva un client, ricordare da quale server è stato accettato.
 Associare ogni Client alla config corretta.
 Usare la config corretta in prepare_response().
 Chiudere correttamente tutti i server fd nel distruttore.
 Testare due server su porte diverse con contenuti diversi.

 DA FARE:
 Gestire errori su un singolo listening socket senza chiudere inutilmente tutto.


**TODO — Request parsing**
Fatto
Gestire richieste ricevute in più recv().
Non considerare completa la request finché non è arrivato tutto il body dichiarato.
Gestire richieste senza body.
Leggere e validare Content-Length.
Rifiutare un Content-Length vuoto, non numerico o troppo lungo.
Imporre un limite alla dimensione degli header.
Richiedere l’header Host.
Validare la request line di base:
metodo presente;
path presente e iniziato da /;
versione nel formato HTTP/...;
niente campi extra nella prima riga.
Accumulare correttamente un body spezzato su più recv().

DA fare.
Controllare client_max_body_size prima di accumulare un body enorme.
Transfer-Encoding: chunked.
Conflitto tra Content-Length e Transfer-Encoding.
Gestione corretta degli header duplicati.
Query string:
separare /pagina?x=1 in path e query;
non cercare sul filesystem un file chiamato pagina?x=1.
Percent encoding, per esempio %20.
Migliorare ulteriormente la validazione della request line.
Timeout per richieste incomplete.
Gestione precisa di disconnessioni ed errori temporanei di recv().

**TODO — Response HTTP**
FATTO:
status line corretta, per esempio HTTP/1.1 200 OK;
Content-Type;
Content-Length calcolato dal body;
header aggiuntivi tramite res.headers;
Connection: close;
redirect con header Location;
pagine di errore configurate;
fallback HTML interno;
gestione degli invii parziali con bytes_sent;
risposta senza body per il redirect.

DA FARE:
aggiungere tutte le reason phrase necessarie;
gestire bene 201, 204, 409;
distinguere gli errori temporanei di send();
controllare eventuali header duplicati;
decidere se mantenere solo Connection: close oppure supportare keep-alive;
confrontare alcuni status e header con NGINX;
verificare che tutte le risposte senza body abbiano davvero Content-Length: 0

**TODO — GET**
FATTO
 Match della location più specifica.
Scelta del root della location, con fallback sul root del server.
Scelta dell’index della location, con fallback sull’index del server.
Costruzione del path reale sul filesystem.
Controllo dell’esistenza del path con stat().
Distinzione tra file regolare e directory.
Lettura e restituzione di file statici.
Gestione delle directory tramite file index.
Gestione autoindex on.
Gestione autoindex off.
Content-Type di base in base all’estensione.
Errori 403, 404 e 500.
Error page personalizzata della location.
Fallback sull’error page personalizzata del server.
Fallback finale sulla pagina HTML interna.
Redirect configurato prima della gestione GET.
Matching corretto tra /old e /old/..., senza confondere /older.
Sanitizzazione basilare dei segmenti . e ..

DA FARE:
Gestire le query string separando:
path, per esempio /index.html;
query, per esempio x=1.
Gestire il redirect delle directory senza slash finale:
/special → /special/.
Validare o decodificare il percent encoding:
per esempio %20.
Migliorare ulteriormente la protezione dal path traversal.
Ampliare i MIME type, se necessario.
Fare un test esplicito dell’error page specifica della location.
Confrontare alcuni comportamenti con NGINX, soprattutto directory e query string.

**TODO — POST e upload**
 Implementare POST.
 Controllare il metodo permesso nella location.
 Usare upload_path.
 Creare file caricati.
 Gestire nomi file in sicurezza.
 Gestire multipart/form-data.
 Gestire body raw quando necessario.
 Restituire codici corretti, per esempio 201.
 Evitare sovrascritture indesiderate.
 Gestire errori disco e permessi.
 Rispettare client_max_body_size.

**TODO — DELETE**
 Implementare DELETE.
 Verificare che il path sia autorizzato.
 Eliminare file regolari.
 Decidere comportamento sulle directory.
 Restituire codici corretti.
 Gestire file inesistenti e permessi.

**TODO — CGI**
 Riconoscere l’estensione configurata.
 Selezionare l’eseguibile CGI corretto.
 Creare pipe non bloccanti.
 Fare fork() solo per CGI.
 Preparare le variabili d’ambiente.
 Passare body e request al CGI.
 Eseguire il CGI nella directory corretta.
 Leggere output CGI tramite epoll.
 Interpretare header CGI.
 Gestire CGI senza Content-Length.
 Gestire timeout e processi bloccati.
 Fare waitpid() senza bloccare il server.
 Gestire errori di execve().

**TODO — Robustezza epoll**
 Gestire EAGAIN/EWOULDBLOCK senza chiudere il client.
 Distinguere recv() == 0 da recv() == -1.
 Non trattare ogni errore temporaneo come disconnessione.
 Gestire EPOLLRDHUP.
 Gestire più accept disponibili per evento.
 Accettare client in ciclo finché accept() non è esaurita.
 Gestire più letture disponibili per evento, se necessario.
 Evitare doppie chiusure dei fd.
 Gestire SIGPIPE.
 Gestire arresto pulito del server.
 Implementare timeout client.
 Evitare request che restano appese indefinitamente.

**TODO — Test finali**
 Test porta singola.
 Test più porte.
 Test più client contemporanei.
 Stress test.
 Test GET statico.
 Test directory e autoindex.
 Test error pages.
 Test redirect.
 Test body troppo grande.
 Test POST/upload.
 Test DELETE.
 Test CGI.
 Test richieste spezzate in più pacchetti.
 Test client che si disconnette.
 Test con browser.
 Test con curl.
 Test con netcat.
 Test con Valgrind.
 Test confronto con NGINX.


# Recap:

**1. Legge il file .conf**

```
file di configurazione
        ↓
Config::parse()
        ↓
std::vector<ServerConfig> configs

Esempio:
configs[0] = server porta 8081, root ./www
configs[1] = server porta 8082, root ./www2
```

**2. Crea i listening socket**
Per ogni configurazione:
```
configs[i]
   ↓
socket()
bind()
listen()
   ↓
listening fd

Poi salva:
listening_sockets[server_fd] = i;

Esempio:
fd 3 → configs[0]
fd 4 → configs[1]
```

**3. Arriva una connessione**
epoll_wait() restituisce un fd.

```
current_fd
    ↓
è dentro listening_sockets?
Se sì, è un socket che sta ascoltando.

Esempio:
current_fd = 3
listening_sockets[3] = 0

Quindi sappiamo:
la connessione sta arrivando sul server configs[0]
```

**4. Accetta il client**

```
client_fd = accept(current_fd, ...);

Esempio:
accept sul fd 3
        ↓
nuovo client fd 7

A quel punto salva due cose:
clients[7] = Client(7);
client_configs[7] = 0;

Quindi:
clients[7] = oggetto Client del fd 7

client_configs[7] = il client 7 usa configs[0]
```

# Schema visivo completo
```txt
CONFIG FILE
    ↓
Config::parse()
    ↓
configs
┌───────────┬────────────────────────┐
│ index 0   │ porta 8081, root ./www │
│ index 1   │ porta 8082, root ./www2│
└───────────┴────────────────────────┘
    ↓
setup socket
    ↓
listening_sockets
┌───────────┬──────────────┐
│ fd 3      │ config 0     │
│ fd 4      │ config 1     │
└───────────┴──────────────┘
    ↓
arriva connessione su fd 3
    ↓
accept()
    ↓
nuovo client fd 7
    ↓
┌────────────────────────────┐
│ clients[7] = Client fd 7   │
│ client_configs[7] = 0      │
└────────────────────────────┘
    ↓
il client fd 7 usa configs[0]
```

# Quando arriva una request
Quando epoll restituisce il fd del client:  
current_fd = 7  
Il server fa:
Client &client = clients[7];  
Poi recupera la configurazione:  
size_t index = client_configs[7];  
ServerConfig &config = configs[index];  

Quindi:
```
clients[7]
    ↓
contiene request, body, response

client_configs[7]
    ↓
restituisce 0

configs[0]
    ↓
contiene root, location, error page, metodi...

Infine:
client.prepare_response(configs[0]);
```

### Esempio finale
```
fd 3 ascolta sulla porta 8081
listening_sockets[3] = 0

accept(fd 3) restituisce fd 7
client_configs[7] = 0
clients[7] = Client(7)

arriva una GET sul fd 7
→ recupero clients[7]
→ recupero client_configs[7]
→ vale 0
→ uso configs[0]
→ cerco file nella root di configs[0]
```
