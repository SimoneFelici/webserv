# webserv
NB: bug nella lettura delle richieste: client_max_body_size viene controllato in validate_req(), ma solamente dopo che parse_body() ha aspettato e memorizzato tutto il body.


**TODO — Pulizia config e valori hardcodati**

/*FATTO
 Rimuovere PORT, ADDRESS e MAX_CONN hardcodati.
 Usare config.address e config.port nel socket.
 Rimuovere version da ServerConfig.
 Rimuovere max_conn dalla config e usare SOMAXCONN.
 Correggere il main e chiamare server.run().
 Sostituire 4096 e 1024 con macro tecniche nominate.
 Applicare client_max_body_size.

 DA FARE:
 Usare le error_page personalizzate del server.
 Usare le error_page personalizzate della location.
 Lasciare la pagina errore HTML interna come fallback.
 Applicare i redirect configurati con return.
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
 Smettere di usare solo configs[0].
 Creare un listening socket per ogni ServerConfig.
 Usare un solo epoll_fd per tutti i server e tutti i client.
 Registrare tutti i listening socket nello stesso epoll.
 Distinguere listening socket e client socket.
 Associare ogni listening socket alla sua ServerConfig.
 Quando arriva un client, ricordare da quale server è stato accettato.
 Associare ogni Client alla config corretta.
 Usare la config corretta in prepare_response().
 Gestire errori su un singolo listening socket senza chiudere inutilmente tutto.
 Chiudere correttamente tutti i server fd nel distruttore.
 Testare due server su porte diverse con contenuti diversi.


**TODO — Request parsing**
 Gestire richieste ricevute in più recv().
 Evitare di aspettare solo \r\n\r\n quando esiste un body.
 Controllare Content-Length prima di accumulare un body enorme.
 Gestire richieste senza body correttamente.
 Gestire Transfer-Encoding: chunked.
 Rifiutare richieste con Content-Length invalido.
 Gestire conflitto tra Content-Length e Transfer-Encoding.
 Gestire header duplicati correttamente.
 Imporre un limite alla dimensione degli header.
 Gestire URL con query string.
 Separare path e query.
 Decodificare o validare il percent encoding.
 Migliorare la validazione della request line.
 Gestire disconnessione durante una request incompleta.
 Gestire timeout per richieste incomplete.

**TODO — Response HTTP**
 Usare reason phrase centralizzate.
 Gestire pagine errore configurate.
 Implementare redirect con header Location.
 Controllare che Content-Length sia sempre corretto.
 Decidere gestione Connection: close e keep-alive.
 Aggiungere header necessari.
 Evitare header duplicati.
 Gestire correttamente risposte senza body.
 Verificare codici HTTP con NGINX.
 Gestire errori di send() non bloccante, inclusi invii parziali.

**TODO — GET**
FATTO
 Match della location più specifica.
 Scelta root server/location.
 Scelta index server/location.
 Gestione autoindex.
 Lettura file statici.
 Content type di base.

DA FARE:
 Gestire redirect delle directory senza slash finale.
 Migliorare protezione dal path traversal.
 Gestire query string senza includerla nel path filesystem.
 Gestire file senza estensione in get_content_type().
 Ampliare MIME types.
 Verificare differenza tra root e comportamento delle location.
 Usare error pages configurate anche negli errori GET.

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

