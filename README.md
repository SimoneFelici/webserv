*This project has been created as part of the 42 curriculum by sfelici, fmontini.*

# Webserv

An HTTP server written in C++98.

---

## Description

A **web server** is a program that listens for incoming connections from clients, such as web browsers, and communicates with them using the **HTTP (Hypertext Transfer Protocol)**.

In a typical HTTP communication, a client establishes a connection with the server and sends an **HTTP request** asking for a resource or requesting an operation. The server processes the request and sends back an **HTTP response**, containing a status code, headers and, when necessary, a response body.

**Webserv** is an implementation of an HTTP server written in **C++98**.
The goal of the project is to understand how client-server communication works at a lower level, from socket connections and HTTP parsing to request processing and response generation.

The server:

* accepts connections from multiple clients;
* parses and interprets HTTP requests;
* generates appropriate HTTP responses and status codes;
* serves static files such as HTML pages, stylesheets and other resources;
* supports the `GET`, `POST` and `DELETE` HTTP methods;
* allows clients to upload files to the server;
* supports **CGI (Common Gateway Interface)**, a standard mechanism that allows the web server to execute an external program or script and use its output to generate dynamic HTTP responses;
* uses **non-blocking I/O**, allowing the server to handle multiple connections without waiting for a single client operation to complete;
* uses a configuration file to define the server behaviour, listening ports, routes and other settings.

The configuration system and the supported directives are described in more detail in the **Configuration** section below.

---

## Instructions

### Compilation

Compile the project using:

```bash
make
```

The project is compiled according to the C++98 standard with the following flags:

```text
-Wall -Wextra -Werror
```

Other available Makefile rules are:

```bash
make clean
make fclean
make re
```

### Execution

The server must be started with a configuration file:

```bash
./webserv <configuration_file>
```

A complete example is provided in the repository:

```bash
./webserv complete_webserv.conf
```

`complete_webserv.conf` is the main example configuration and demonstrates a setup serving two different websites.

Other configuration files are also included for testing specific behaviours and features.

The configuration files can be modified or new ones can be created according to the supported directives. Their syntax and available options are explained in the **Configuration** section below.

---

## Configuration

Webserv behaviour is defined through a configuration file passed to the executable at startup.

For example:

```bash
./webserv complete_webserv.conf
```

The repository includes `complete_webserv.conf`, a complete example configuration that defines two different servers listening on different ports.

Configuration files use a syntax inspired by NGINX and are composed of one or more `server` blocks.
Each server can contain general directives and one or more `location` blocks used to define specific behaviour for particular URL paths.

### Basic structure

A configuration file has the following general structure:

```conf
server {
    listen 0.0.0.0:8081;
    server_name example_server;

    root ./www;
    index index.html;
    autoindex off;

    client_max_body_size 1000000;
    allowed_methods GET POST DELETE;

    location / {
        root ./www;
        index index.html;
        autoindex off;
        allowed_methods GET;
    }
}
```

Each directive ends with a semicolon (`;`), while `server` and `location` blocks are enclosed in curly brackets.

Multiple `server` blocks can be defined in the same configuration file.

---

### Server block

A `server` block defines a server instance and its general behaviour.

Example:

```conf
server {
    listen 0.0.0.0:8081;
    server_name main_webserv;

    root ./www;
    index index.html;
    autoindex off;

    client_max_body_size 1000000;
    allowed_methods GET POST DELETE;
}
```

The main supported directives are:

| Directive              | Description                                                          |
| ---------------------- | -------------------------------------------------------------------- |
| `listen`               | Defines the interface and port on which the server listens.          |
| `server_name`          | Defines a name for the server configuration.                         |
| `root`                 | Defines the base directory used to search for resources.             |
| `index`                | Defines the default file served when a directory is requested.       |
| `autoindex`            | Enables or disables automatic directory listing.                     |
| `client_max_body_size` | Defines the maximum accepted size of an HTTP request body, in bytes. |
| `allowed_methods`      | Defines the HTTP methods accepted by the server or route.            |
| `error_page`           | Associates an HTTP error status code with a custom error page.       |

---

### Locations

A `location` block defines specific rules for a URL path.

For example:

```conf
location /public {
    root ./www/public;
    index index.html;
    autoindex on;
    allowed_methods GET;
}
```

Requests matching `/public` are handled according to the directives defined for that location.

Different locations can therefore expose different resources and behaviours.

For example, the provided configuration uses separate locations for static content, file uploads, uploaded files, redirects and CGI execution.

---

### Allowed HTTP methods

The `allowed_methods` directive specifies which HTTP methods are accepted.

Example:

```conf
allowed_methods GET POST DELETE;
```

It can also be defined for a specific location:

```conf
location / {
    allowed_methods GET;
}
```

or:

```conf
location /uploads {
    allowed_methods GET DELETE;
}
```

This makes it possible to restrict different routes to different operations.

---

### Request body size

The maximum accepted request body size is configured with:

```conf
client_max_body_size 1000000;
```

The value represents the maximum body size in bytes.

It can also be configured for a specific location when that route requires a different limit.

For example:

```conf
location /post_body {
    allowed_methods POST;
    upload_path ./YoupiBanane;
    client_max_body_size 100;
}
```

This allows a route to apply a more restrictive request body limit than the general server configuration.

---

### Custom error pages

Custom error pages can be associated with HTTP status codes using the `error_page` directive.

Example:

```conf
error_page 400 ./www/errors/400.html;
error_page 403 ./www/errors/403.html;
error_page 404 ./www/errors/404.html;
error_page 405 ./www/errors/405.html;
error_page 413 ./www/errors/413.html;
error_page 500 ./www/errors/500.html;
error_page 501 ./www/errors/501.html;
error_page 505 ./www/errors/505.html;
```

If a custom page is configured for a status code, Webserv can use the corresponding file when generating the error response.

---

### Directory listing

Directory listing is controlled through the `autoindex` directive.

To disable it:

```conf
autoindex off;
```

To enable it:

```conf
location /public {
    root ./www/public;
    index index.html;
    autoindex on;
    allowed_methods GET;
}
```

When enabled, Webserv can generate a directory listing when the requested resource is a directory and no appropriate index file is served.

---

### File uploads

A route intended to receive uploaded files can define an `upload_path`.

Example:

```conf
location /upload {
    allowed_methods POST;
    upload_path ./www/uploads;
}
```

In this case, POST requests to `/upload` can store uploaded files inside:

```text
./www/uploads
```

A different route can then expose or manage those files:

```conf
location /uploads {
    root ./www/uploads;
    autoindex on;
    allowed_methods GET DELETE;
}
```

---

### HTTP redirection

A location can return an HTTP redirect using the `return` directive.

Example:

```conf
location /old {
    allowed_methods GET;
    return 301 /;
}
```

A request to `/old` therefore generates a `301` redirect to `/`.

---

### CGI

CGI execution is configured by associating a file extension with an executable.

For example:

```conf
location /cgi-bin {
    root ./www/cgi-bin;
    index index.py;
    autoindex off;
    allowed_methods GET POST;
    cgi .py /usr/bin/python3;
}
```

The directive:

```conf
cgi .py /usr/bin/python3;
```

associates `.py` files with the Python interpreter.

When a matching resource is requested, Webserv can execute the CGI program instead of serving the file as ordinary static content. The CGI process receives information about the HTTP request and its output is used by the server to build the HTTP response.

---

### Multiple servers

A single configuration file can contain multiple `server` blocks.

The provided `complete_webserv.conf`, for example, defines a main server on port `8081`:

```conf
server {
    listen 0.0.0.0:8081;
    server_name main_webserv;

    ...
}
```

and a second server on port `8082`:

```conf
server {
    listen 0.0.0.0:8082;
    server_name secondary_webserv;

    root ./www2;
    index index.html;
    autoindex off;

    client_max_body_size 2000000;
    allowed_methods GET;

    location / {
        root ./www2;
        index index.html;
        autoindex off;
        allowed_methods GET;
    }
}
```

The two servers can therefore listen simultaneously on different ports and serve different content.

---

### Creating a custom configuration

To create a new configuration file:

1. Define at least one `server` block.
2. Choose the interface and port with `listen`.
3. Define the directory containing the website with `root`.
4. Define an optional default file with `index`.
5. Configure the accepted HTTP methods with `allowed_methods`.
6. Set the desired request body limit with `client_max_body_size`.
7. Add `location` blocks when different URL paths require different behaviour.
8. Configure optional features such as uploads, redirects, custom error pages, directory listing or CGI execution.
9. Save the configuration file and pass it to Webserv:

```bash
./webserv my_config.conf
```

The provided `complete_webserv.conf` can be used as a reference and modified to create additional server configurations.

---

## Technical Choices

### Event-driven architecture

Webserv follows an **event-driven architecture** designed to handle multiple client connections without creating one thread or one process for each client.

Instead of waiting for a single connection to finish before processing another one, the server keeps track of multiple file descriptors and reacts only when an operation can actually be performed.

This approach allows a single server process to manage several clients concurrently while avoiding blocking operations on network sockets.

---

### epoll

The server uses **`epoll`** as the I/O multiplexing mechanism.

`epoll` allows the program to monitor multiple file descriptors through a single event loop and receive notifications when one of them becomes ready for an operation.

A single `epoll` instance is used to monitor the file descriptors involved in server communication, including:

* listening sockets;
* connected client sockets;
* descriptors that need to be monitored for readable events;
* descriptors that need to be monitored for writable events.

The main event loop is therefore responsible for deciding what action must be performed depending on the type of descriptor and the event reported by `epoll`.

Conceptually, the server works as follows:

```text
                    ┌─────────────────┐
                    │   epoll_wait()  │
                    └────────┬────────┘
                             │
                       ready events
                             │
               ┌─────────────┴─────────────┐
               │                           │
        listening socket              client socket
               │                           │
            accept()              ┌────────┴────────┐
                                  │                 │
                              readable          writable
                                  │                 │
                               recv()             send()
                                  │                 │
                           parse request      send response
```

This avoids continuously checking every connected client and allows the server to react only to descriptors that are ready.

---

### Non-blocking sockets

Network file descriptors are configured in **non-blocking mode**.

This is essential because a blocking `recv()` or `send()` could stop the entire server while waiting for a single client.

With non-blocking I/O, the server does not assume that an entire HTTP request can be received at once or that an entire response can be sent in one operation.

Instead, incoming and outgoing data can be processed progressively across multiple `epoll` events.

---

### Client state

Each connected client has its own state.

The server keeps the data associated with a connection between different iterations of the event loop, allowing a request to be reconstructed even when it arrives in several network reads.

A client can therefore move through different phases such as:

```text
connection accepted
        ↓
receiving request
        ↓
parsing HTTP data
        ↓
processing request
        ↓
building response
        ↓
sending response
        ↓
connection closed
```

This is particularly important because TCP is a byte stream: a complete HTTP request is not guaranteed to arrive in a single `recv()` call.

For example, the server may first receive:

```text
POST /upload HTTP/1.1
Host: localhost
Content-Length: 100

first part of the body...
```

and receive the remaining body only during a later readable event.

The request is processed only when enough information has been received to determine that it is complete.

---

### Reading and writing

Read and write operations on network descriptors are performed only after `epoll` reports that the corresponding descriptor is ready.

Readable events are used to receive incoming client data, while writable events are used when a response is ready to be sent.

The server also handles **partial reads and partial writes**.

A call to `recv()` is not expected to contain the entire request, and similarly a call to `send()` is not assumed to transmit the complete response.

Data that has not yet been received or sent remains associated with the client and is processed during a later event.

This prevents slow clients or large requests and responses from blocking the rest of the server.

---

### HTTP request processing

Once a complete HTTP request has been received, Webserv separates the network layer from the actual request handling.

The processing flow can be summarized as:

```text
TCP connection
      ↓
raw HTTP data
      ↓
HTTP request parsing
      ↓
configuration / location matching
      ↓
method handling
      ↓
GET / POST / DELETE / CGI
      ↓
HTTP response generation
      ↓
response sent to the client
```

The parsed request contains the information required by the rest of the server, such as:

* HTTP method;
* requested URI;
* headers;
* request body;
* information related to body encoding.

The server then determines which configuration and `location` apply to the requested resource before executing the corresponding operation.

---

### Configuration-driven behaviour

Webserv is designed so that server behaviour is not tied directly to hard-coded URL paths.

The configuration file defines properties such as:

* listening addresses and ports;
* document roots;
* allowed HTTP methods;
* index files;
* directory listing;
* maximum request body size;
* custom error pages;
* upload directories;
* redirects;
* CGI associations.

When a request is received, the server resolves the matching server and location configuration and uses it to determine how the request must be handled.

This makes the same executable capable of serving different websites and applying different rules without recompilation.

The configuration format is described in detail in the **Configuration** section.

---

### Static files and filesystem operations

Static resources are resolved from the configured `root` and the requested URI.

Filesystem information is inspected to determine whether a requested resource:

* exists;
* is a regular file;
* is a directory;
* can be served;
* requires an index file;
* can produce an autoindex listing.

Regular disk files are treated differently from network sockets: they do not require readiness monitoring through `epoll`, while operations that may wait for network data remain controlled by the event loop.

---

### CGI

Dynamic resources can be handled through **CGI (Common Gateway Interface)**.

When a configured file extension matches a CGI rule, Webserv executes the corresponding interpreter or CGI program and provides the request information required by that process.

CGI makes it possible to generate dynamic content instead of returning only files already stored on disk.

For example, the provided configuration associates Python files with the Python interpreter:

```conf
cgi .py /usr/bin/python3;
```

The result produced by the CGI program is then used by Webserv to construct the HTTP response returned to the client.

---

### Why this architecture?

The main objective of these choices is to keep the server responsive even when several clients are connected at the same time.

Using:

* one event-driven loop;
* `epoll`;
* non-blocking sockets;
* per-client state;
* incremental reads and writes;
* configuration-driven routing;

allows Webserv to handle concurrent HTTP connections without dedicating a separate execution flow to every client.

It also reflects one of the central goals of the project: understanding how a web server manages HTTP communication below the abstractions normally provided by web frameworks and existing server software.

---

## Supported methods

- `GET` — retrieves static resources or directory content.
- `POST` — sends data to the server and supports file uploads.
- `DELETE` — removes resources when allowed by the route configuration.

---

## CGI

DA RIVEDERE
The server supports CGI execution based on file extensions configured
in the server configuration.

CGI processes receive request information through environment variables
and request bodies through standard input.

---
## Resources

The following resources were consulted during the development of Webserv to better understand networking, HTTP communication, web server behaviour and non-blocking I/O.

### Networking and sockets

* [C++ Network Programming Part 1: Sockets](https://www.youtube.com/watch?si=P3pQhLMq3-GRRHkX&v=gntyAFoZp-E&feature=youtu.be)
  Used to study the fundamentals of socket programming and client-server communication, including IP addresses, ports and the basic lifecycle of a server socket.

* [99% of Developers Don't Get Sockets](https://www.youtube.com/watch?si=5Q5uhb0Ldp4YK7Ic&v=D26sUZ6DHNQ&feature=youtu.be)
  Used to reinforce the conceptual understanding of sockets and how network communication works between clients and servers.

### Web servers and configuration

* [Web Server Concepts and Examples](https://www.youtube.com/watch?v=9J1nJOivdyw)
  Used as an introductory resource to understand the role of a web server and the general client-server communication model.

* [NGINX Beginner's Guide](https://nginx.org/en/docs/beginners_guide.html)
  Used as a reference for web server behaviour and configuration structure. In particular, it helped in understanding the organization of `server` and `location` blocks, serving static content and the general approach used by NGINX configuration files.

### Non-blocking I/O and epoll

* [Non-Blocking Sockets and I/O Multiplexing with epoll in C](https://www.ablt.dev/blog/nonblocking-sockets)
  Used to study the difference between blocking and non-blocking sockets, I/O multiplexing and the `epoll` event model, including readable and writable events.

### HTTP status codes

* [HTTP Cats](https://http.cat)
  Used as a quick visual reference for HTTP response status codes while implementing and testing server responses and error handling.

### 42 peer references

Webserv repositories from other 42 students were also consulted during the project as additional learning material.

They were used to compare different approaches to project organization, server architecture and solutions to specific problems encountered during development. These repositories were treated as references for study and comparison; implementation choices were reviewed, understood and adapted to the requirements and architecture of this project.

### AI usage

AI tools were used as learning and support resources throughout the project.

They were used to:

* clarify HTTP concepts, client-server communication and protocol behaviour;
* better understand socket programming and non-blocking I/O;
* reason about the use of `epoll` and the event-driven architecture of the server;
* better understand HTTP request parsing and response generation;
* study and review the implementation logic of the `GET`, `POST` and `DELETE` methods;
* understand `multipart/form-data` requests and file uploads;
* understand and reason about chunked transfer encoding and request body decoding;
* discuss edge cases and possible server behaviours;
* design and review testing strategies using different types of HTTP requests;
* support debugging by analysing unexpected behaviours and possible causes;
* review the configuration system and its expected behaviour;
* review and improve the project documentation and README.

AI-generated explanations and implementation suggestions were not used without review. They were analysed, tested and adapted before being integrated into the project, and only solutions that could be understood and explained by the authors were retained.

---

### Documentation

- HTTP RFC documentation
- MDN Web Docs — HTTP
- NGINX documentation
- Linux man pages:
  - socket(2)
  - bind(2)
  - listen(2)
  - accept(2)
  - recv(2)
  - send(2)
  - epoll(7)
  - fcntl(2)
- CGI documentation

---

### AI usage

AI tools were used as a learning and support resource during the project.

They were used to:

- clarify HTTP concepts and protocol behaviour;
- better understand request and response parsing;
- study GET, POST and DELETE request handling;
- understand multipart/form-data and chunked transfer encoding;
- discuss edge cases and possible tests;
- review debugging strategies;
- better understand non-blocking I/O and epoll behaviour;
- review and improve project documentation.

AI-generated suggestions were reviewed, tested and adapted before being
used in the project.


## Testing?
