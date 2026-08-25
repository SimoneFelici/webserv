# Mandatory Part

*Check the code and ask questions*

- Install Siege using Homebrew.  
- Ask for an explanation about the basics of an HTTP server.  
- Ask which function the group used for I/O multiplexing.  
- Ask for an explanation of how select() (or equivalent) works.  
- Ask if they use only one select() (or equivalent) and how they handle server acceptance and client read/write operations.  
- The select() (or equivalent) should be in the main loop and must check file descriptors for both reading and writing simultaneously.   Otherwise, the grade is 0, and the evaluation process ends immediately.
- There should be only one read or one write per client per select() (or equivalent). Ask the group to show you the code from the select() (or equivalent) to the read and write of a client.
- Search for all read/recv/write/send on a socket and check that, if an error is returned, the client is removed.
- Search for all read/recv/write/send and check if the returned value is correctly checked (checking only -1 or 0 values is not enough, both should be checked).
- f errno is checked after read/recv/write/send, the grade is 0 and the evaluation process ends immediately (errno can be used for logging info only).
- Writing or reading ANY file descriptor without going through the select() (or equivalent) is strictly FORBIDDEN.
- The project must compile without any relinking issues. If not, use the 'Invalid compilation' flag.
- If any point is unclear or is not correct, the evaluation stops.

 Yes   No

---

## Configuration
In the configuration file, check whether you can do the following and
test the result:

- Search for the HTTP response status codes list on the internet. During this evaluation, if any status codes are incorrect, do not award any points related to them.
- Set up multiple websites on different interfaces and ports.
- Set up a default error page (try modifying the 404 error page).
- Limit the size of the client request body (use: curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit").
- Set up routes on a server to different directories.
- Set up a default file to serve when requesting a directory.
- Set up a list of accepted methods for a specific route (e.g., try DELETE something with and without permission).

 Yes   No

```bash
TESTS:
* curl -i http://127.0.0.1:8081/questo-file-non-esiste

* con client_max_body_size 10:

REQUEST SOTTO LIMITe: (HTTP/1.1 201 Created)
curl -i -X POST \
  -H "Content-Type: plain/text" \
  --data "ciao" \
  http://127.0.0.1:8081/upload/short.txt

  REQUEST SOPRA LIMITE: (HTTP/1.1 413 Payload Too Large)
curl -i -X POST \
  -H "Content-Type: plain/text" \
  --data "questo-body-supera-dieci-byte" \
  http://127.0.0.1:8081/upload/long.txt

BODY DA 101:
curl -i -X POST -H "Content-Type: plain/text" --data "$(python3 -c 'print("A"*101, end="")')" http://127.0.0.1:8081/upload/body101.txt

operazioni consentite per route 
curl -i -X DELETE http://127.0.0.1:8081/public/ (201 ok)
curl -i -X DELETE http://127.0.0.1:8081/public/ (405 Method Not Allowed)

```

---

## Basic checks

Using telnet, curl, and pre-prepared files, demonstrate that the
following features function correctly:

GET, POST, and DELETE requests should work.
UNKNOWN requests should not result in a crash.
For every test, you should receive the appropriate status code.
Upload some files to the server and retrieve them.

 Yes   No

TEST:
```bash
telnet 127.0.0.1 8081

GET /questo-non-esiste HTTP/1.1
Host: localhost

---
telnet 127.0.0.1 8081
GET / HTTP/1.1

```

--- 

## Check CGI

Pay attention to the following:

The server should function correctly with CGI.
The CGI should be run in the correct directory for relative path file access.
With the help of the student(s), you should check that everything is working properly. You must test the CGI with the "GET" and "POST" methods.
You must test with CGI files containing errors to ensure that server's error handling works correctly. You can use a script containing an infinite loop or an error; you are free to do whatever tests you want within the limits of acceptability that remain at your discretion. The group being evaluated should help you with this.
The server should never crash, and an error should be displayed if an issue occurs.

 Yes   No

---

## Check with a browser

Use the browser chosen by the team. Open the network tab and try connecting to the server.
Look at the request header and response header.
It should be compatible with serving a fully static website.
Try an incorrect URL on the server.
Try to list a directory.
Try a redirected URL.
Try anything you like.

 Yes   No

---

## Port issues

In the configuration file, set up multiple interfaces and ports to provide different websites. Use the browser to verify that the configuration works correctly and serves the appropriate website.
Configure multiple websites on the same interface:port. This should result in an error, unless the team has chosen to implement the virtual host feature. Both approaches are valid, as long as everything works as expected.
Launch multiple webserv programs at the same time with different configuration files but with common interface:ports. Does it work? If it does, ask why. Ensure that, whatever the group's choice was, the program behaviour is coherent and does not crash.

 Yes   No

---

## Siege & stress test

Use Siege to run some stress tests.
Availability should be above 99.5% for a simple GET request on an empty page using a siege with the -b flag.
Ensure there are no memory leaks (Monitor the process memory usage; it should not increase indefinitely).
Check that there are no hanging connections.
You should be able to use siege indefinitely without having to restart the server (take a look at siege with -b flag).
When conducting load tests using the siege command, be careful, it depends on your OS. it is crucial to limit the number of connections per second by specifying options such as -c (number of clients), -d (maximum wait time before a client reconnects), and -r (number of attempts). The choice of these parameters is at the evaluator's discretion. However, it is imperative to reach an agreement with the person being evaluated to ensure a fair and transparent assessment of the web server's performance.

 Yes   No