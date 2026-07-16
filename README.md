# webserv

to do get controlli finali nel parsing:
Verificare che esista almeno un blocco server.
Verificare che ogni server abbia almeno listen.
Decidere valori di default per campi mancanti:
root
index
allowed_methods
client_max_body_size
autoindex
Verificare eventuali coppie duplicate address:port tra più server.
Decidere definitivamente la sintassi ufficiale: allowed_methods, upload_path, cgi.
Fermare il programma quando parse_config() restituisce false.