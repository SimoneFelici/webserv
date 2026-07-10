# webserv

to do get:
1. parser config vero
   ora autoindex è hardcoded false, quindi non puoi testare autoindex da conf/example.conf

2. error pages configurate
   build_error_response() genera solo HTML hardcoded

3. redirect configurato nella location
   il subject richiede HTTP redirection nelle route

4. CGI
   non è solo GET statica, ma il progetto lo richiede