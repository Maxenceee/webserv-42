# Webserv

My Webserv project for the 42 School cursus, a [`Nginx`](https://nginx.org) like web server made in c++98. This project is much more complete and complex than what is required by the subject to be validated at 125. It includes many Nginx directives and advanced features such as proxies. See the [Features](#features) section below.

Final grade : `125/100` & Outstanding Project

## Getting started

The server can be used in the classic way, or can be daemonized via [launchd](https://developer.apple.com/library/archive/documentation/MacOSX/Conceptual/BPSystemStartup/Chapters/CreatingLaunchdJobs.html) (OS X) or [systemd](https://man7.org/linux/man-pages/man1/systemd.1.html) (Linux).

To start the server:

``` sh
$> ./webserv [config_file]
```

Use `-d` or `--debug` parameter to enable debug mode.

To daemonize the server:

``` sh
$>  ./setup_daemon.bash
```

In this case you need to place your config file in `/etc/webserv` with the name `webserv.conf`.

On MacOSX log file will be `/var/log/webserv.log` and `/var/log/webserv.err`.

# Basic concept

The server is based on a simple state machine. It reads the configuration file and creates a list of servers. Each server has a list of locations. Each location has a list of directives. When a request is received, the server will try to match the request URI with the locations of the server. If a location is found, the server will apply the directives of the location to the request.

## Overview

1. **Initialization**: When the server starts, it reads its configuration settings and prepares to handle requests. This includes setting up necessary resources and initializing server components.
2. **Listening for Requests**: The server waits for incoming requests from users. It uses network sockets to listen for these requests on specified ports.
3. **Processing Requests**: When a request arrives, the server processes it. This involves parsing the request, determining the appropriate action, and preparing the response.
4. **Sending Responses**: After processing the request, the server sends back the appropriate response to the user. This could be the content of a web page, a file, or an error message if the request could not be fulfilled.
5. **Handling Multiple Requests**: The server can manage multiple requests at the same time using efficient I/O operations and a thread pool to ensure quick and reliable service.
6. **Graceful Shutdown**: When it’s time to shut down, the server stops accepting new requests and finishes processing any remaining ones. This ensures that all ongoing processes are completed smoothly.

## How It Works

### Server Initialization

The server initialization involves setting up the necessary components to start listening for requests. This includes creating a socket, setting socket options, and binding the socket to a specific port and address.

#### Creating a Socket

The `socket()` function is used to create a network socket, which serves as an endpoint for communication. The socket is created using IPv4 (AF_INET) or IPv6 (AF_INET6) and TCP (SOCK_STREAM), which ensures reliable, connection-oriented communication.

``` c++
int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
```

#### Binding the Socket

Binding a socket is a crucial step in setting up a server as it associates the socket with a specific IP address and port on the local machine. This allows the server to listen for incoming connections on that address and port. Here’s a more detailed look at the structures and functions involved in the binding process.

``` c++
// IPv4 Address Structure
struct sockaddr_in {
    sa_family_t    sin_family   // Address family (AF_INET for IPv4)
    in_port_t      sin_port;    // Port number (16 bits)
    struct in_addr sin_addr;    // Internet address (32 bits)
    char           sin_zero[8]; // Padding to match size of sockaddr
};
```
``` c++
// IPv6 Address Structure
struct sockaddr_in6 {
    sa_family_t     sin6_family;   // Address family (AF_INET6 for IPv6)
    in_port_t       sin6_port;     // Port number (16 bits)
    uint32_t        sin6_flowinfo; // IPv6 flow information
    struct in6_addr sin6_addr;     // IPv6 address (128 bits)
    uint32_t        sin6_scope_id; // IPv6 scope ID (interface index)
};
```

The `bind()` function assigns the address specified in the sockaddr_in structure to the socket. This step is necessary for the socket to accept incoming connections on the specified IP address and port.

``` c++
struct sockaddr_in socket_addr;

bzero(&socket_addr, sizeof(socket_addr));           // Clear the structure
socket_addr.sin_family = AF_INET;                   // Set address family to IPv4
socket_addr.sin_port = htons(80);                   // Convert port number to network byte order
socket_addr.sin_addr.s_addr = htonl(IN_ANY_LOCAL);  // Convert IP address to network byte order

bind(socket_fd, (sockaddr *)&socket_addr, sizeof(socket_addr));
```

#### Listening for Connections

The `listen()` function marks the socket as a passive socket that will be used to accept incoming connection requests. It specifies the maximum number of pending connections that can be queued.

``` c++
listen(socket_fd, 1024);
```

#### Accepting Connections

The `accept()` function accepts an incoming connection from a client. It returns a new socket file descriptor for the established connection, allowing the server to communicate with the client.

``` c++
int client_fd = accept(socket_fd, (sockaddr *)&client_addr, &client_addr_len);
```

#### Polling

The `poll()` function is crucial for handling multiple connections simultaneously. It monitors an array of file descriptors to see if any are ready for reading, writing, or have encountered an error. This allows the server to efficiently manage multiple clients without blocking on a single connection.

``` c++
struct pollfd fds[2];
fds[0].fd = socket_fd;
fds[0].events = POLLIN;
fds[1].fd = client_fd;
fds[1].events = POLLIN;

poll(fds, 2, -1);

if (fds[0].revents & POLLIN)
{
    // Accept new connection
}

if (fds[1].revents & POLLIN)
{
    // Handle client request
}
```

### Managing Multiple Clients

To handle multiple clients, the server maintains a list of active connections and uses efficient I/O operations:

- **Tracking Connections**: The server keeps track of all active connections using data structures like `pollfd` and `std::vector`. Each connection is monitored for specific events (e.g., readiness to read data).
``` c++
std::vector<pollfd> poll_fds;
```

- **Processing Events**:  When `poll()` indicates that a socket is ready, the server processes the corresponding event. This could involve accepting a new connection, reading data from a client, or handling a client's request.
```c++
for (size_t i = 0; i < poll_fds.size(); ++i)
{
    if (poll_fds[i].revents & POLLHUP)
    {
        // Connection closed by the client
    }
    else if (poll_fds[i].revents & POLLIN)
    {
        // Read data from the client
    }
}
```

### Let's combine it all

``` c++
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <vector>
#include <iostream>

#define MAX_EVENTS 1024
#define PORT 3000

int main() {
    // Create socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int option = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1)
    {
        perror("setsockopt");
        return (EXIT_FAILURE);
    }

    // Prepare sockaddr_in structure
    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = INADDR_ANY;
    socket_addr.sin_port = htons(PORT);

    // Bind socket to socket_addr and port
    if (bind(socket_fd, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(socket_fd, MAX_EVENTS) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    std::vector<pollfd> poll_fds;

    // Add server socket to poll set
    poll_fds.push_back((pollfd){socket_fd, POLLIN, 0});

    while (true)
    {
        // We cannot remove elements from poll_fds while iterating over it
        std::vector<int>	to_remove;

        if (poll(poll_fds.data(), poll_fds.size(), -1) < 0)
        {
            perror("Poll failed");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < poll_fds.size(); ++i)
        {
            if (poll_fds[i].revents & POLLIN)
            {
                if (poll_fds[i].fd == socket_fd)
                {
                    // Accept new connection
                    int new_socket_fd = accept(socket_fd, NULL, NULL);
                    if (new_socket_fd < 0)
                    {
                        perror("Accept failed");
                        exit(EXIT_FAILURE);
                    }

                    printf("New connection accepted\n");

                    // Add new client to poll set
                    poll_fds.push_back((pollfd){new_socket_fd, POLLIN, 0});
                }
                else
                {
                    // Read data from client
                    char buffer[1024] = {0};
                    int valread = read(poll_fds[i].fd, buffer, sizeof(buffer));
                    if (valread == 0)
                    {
                        // Client disconnected
                        printf("Client disconnected\n");
                        close(poll_fds[i].fd);
                        to_remove.push_back(i);
                    }
                    else
                    {
                        printf("Received: %s\n", buffer);
                    }
                }
            }
        }

        for (std::vector<int>::reverse_iterator it = to_remove.rbegin(); it != to_remove.rend(); it++)
        {
            poll_fds.erase(poll_fds.begin() + *it);
        }
    }

    return 0;
}
```

# Features

## Table of contents

- [add_header](#add_header)
- [alias](#alias)
- [allow_methods](#allow_methods)
- [autoindex](#autoindex)
- [client_body_timeout](#client_body_timeout)
- [client_header_timeout](#client_header_timeout)
- [client_max_body_size](#client_max_body_size)
- [error_page](#error_page)
- [fastcgi_param](#fastcgi_param)
- [fastcgi_pass](#fastcgi_pass)
- [index](#index)
- [listen](#listen)
- [location](#location)
- [proxy_hide_header](#proxy_hide_header)
- [proxy_pass](#proxy_pass)
- [proxy_pass_header](#proxy_pass_header)
- [proxy_set_header](#proxy_set_header)
- [return](#return)
- [root](#root)
- [server](#server)
- [server_name](#server_name)

## Documentation

Based on Nginx documentation ;)

### `add_header`

```
Syntax:		add_header name value [always];
Default:	—
Context:	server, location
```

Adds the specified field to a response header provided that the response code equals 200, 201, 204, 206, 301, 302, 303, 304, 307 or 308. 

There could be several **add_header** directives. These directives are inherited from the previous configuration level if and only if there are no **add_header** directives defined on the current level.

If the **always** parameter is specified, the header field will be added regardless of the response code.

### `alias`

```
Syntax:		alias path;
Default:	—
Context:	location
```

Defines a replacement for the specified location. For example, with the following configuration
```
location /i/ {
    alias /data/w3/images/;
}
```
on request of **“/i/top.gif”**, the file **/data/w3/images/top.gif** will be sent.

When location matches the last part of the directive’s value, it is better to use the [root](#root) directive instead

### `allow_methods`

```
Syntax:		allow_methods method ...;
Default:	—
Context:	server, location
```

Allows you to specify the list of accepted methods. in the case where the request method is not part of the list, the server responds with a 405 (Method Not Allowed) error.

### `autoindex`

```
Syntax:		autoindex on | off;
Default:	autoindex off;
Context:	http, server, location
```

Enables or disables the directory listing output.

### `client_body_timeout`

```
Syntax:	    client_body_timeout time;
Default:	client_body_timeout 60s;
Context:	server, location
```

Defines a timeout for reading client request body. The timeout is set only for a period between two successive read operations, not for the transmission of the whole request body. If a client does not transmit anything within this time, the request is terminated with the 408 (Request Time-out) error.

### `client_header_timeout`

```
Syntax:	    client_header_timeout time;
Default:	client_header_timeout 60s;
Context:	server, location
```

Defines a timeout for reading client request header. If a client does not transmit the entire header within this time, the request is terminated with the 408 (Request Time-out) error.

### `client_max_body_size`

```
Syntax:		client_max_body_size size;
Default:	client_max_body_size 1m;
Context:	server, location
```

Sets the maximum allowed size of the client request body. If the size in a request exceeds the configured value, the 413 (Request Entity Too Large) error is returned to the client. Please be aware that browsers cannot correctly display this error. Setting size to 0 disables checking of client request body size.

### `error_page`

```
Syntax:		error_page code ... uri;
Default:	—
Context:	server, location
```

Defines the URI that will be shown for the specified errors.

### `fastcgi_param`

```
Syntax:		fastcgi_param parameter value [if_not_empty];
Default:	—
Context:	server, location
```

Sets a **parameter** that should be passed to the FastCGI server. The **value** can contain text. These directives are inherited from the previous configuration level if and only if there are no **fastcgi_param** directives defined on the current level.

### `fastcgi_pass`

<!-- Syntax:		fastcgi_pass address; -->
```
Syntax:		fastcgi_pass path;
Default:	—
Context:	location
```

<!-- Sets the address of a FastCGI server. The address can be specified as a domain name or IP address, and a port: -->
Sets the path of a FastCGI executable.
```
fastcgi_pass /usr/bin/php-cgi;
```

### `index`

```
Syntax:		index file ...;
Default:	index index.html;
Context:	server, location
```

Defines files that will be used as an index. Files are checked in the specified order.

### `listen`

```
Syntax:		listen address[:port]
Default:	listen *:80 | *:8000;
Context:	server
```

Sets the address and port for IP on which the server will accept requests. Both address and port, or only address or only port can be specified. An address may also be a hostname, for example:

```
listen 127.0.0.1:8000;
listen 127.0.0.1;
listen 8000;
listen localhost:8000;
```

If only address is given, the port 80 is used.

If the directive is not present then either **\*:80** is used if nginx runs with the superuser privileges, or **\*:8000** otherwise.

If none of the directives have the server_name set to **_** then the first server with the address:port pair will be the default server for this pair.

<!-- If the directive is not present then either *:80 is used if nginx runs with the superuser privileges, or *:8000 otherwise -->

### `location`

```
Syntax:		location [ = | ~ | ~* | ^~ ] uri { ... }
Default:	—
Context:	server, location
```

Sets configuration depending on a request URI.

A location can either be defined by a prefix string, or by a regular expression. Regular expressions are specified with the preceding **“~*”** modifier (for case-insensitive matching), or the **“~”** modifier (for case-sensitive matching). The regular expressions are checked, in the order of their appearance in the configuration file. The search of regular expressions terminates on the first match, and the corresponding configuration is used. 

**location** blocks can be nested, with some exceptions mentioned below.

Also, using the **“=”** modifier it is possible to define an exact match of URI and location. If an exact match is found, the search terminates. For example, if a **“/”** request happens frequently, defining **“location = /”** will speed up the processing of these requests, as search terminates right after the first comparison. Such a location cannot obviously contain nested locations.

Let’s illustrate the above by an example:

```
location = / {
    [ configuration A ]
}

location / {
    [ configuration B ]
}

location /documents/ {
    [ configuration C ]
}

location ^~ /images/ {
    [ configuration D ]
}

location ~* \.(gif|jpg|jpeg)$ {
    [ configuration E ]
}
```

The **“/”** request will match configuration A, the **“/index.html”** request will match configuration B, the **“/documents/document.html”** request will match configuration C, the **“/images/1.gif”** request will match configuration D, and the **“/documents/1.jpg”** request will match configuration E.

### `proxy_hide_header`

```
Syntax:     proxy_hide_header field;
Default:	—
Context:	server, location
```

By default, the server does not pass the header fields “Date”, “Server”, “X-Pad”, and “X-Accel-...” from the response of a proxied server to a client. The **proxy_hide_header** directive sets additional fields that will not be passed. If, on the contrary, the passing of fields needs to be permitted, the [proxy_pass_header](#proxy_pass_header) directive can be used.

### `proxy_pass`

```
Syntax:		proxy_pass URL;
Default:	—
Context:	location
```

Sets the protocol and address of a proxied server and an optional URI to which a location should be mapped. Only “http” protocol is supported for now. The address can be specified as a domain name or IP address, and an optional port:

```
proxy_pass http://localhost:8000;
```

If a domain name resolves to several addresses, all of them will be used in order.

<!-- If the **proxy_pass** directive is specified with a URI, then when a request is passed to the server, the part of a normalized request URI matching the location is replaced by a URI specified in the directive:

```
location /name/ {
    proxy_pass http://127.0.0.1/remote/;
}
```

If **proxy_pass** is specified without a URI, the request URI is passed to the server in the same form as sent by a client when the original request is processed, or the full normalized request URI is passed when processing the changed URI:

```
location /some/path/ {
    proxy_pass http://127.0.0.1;
}
``` -->

### `proxy_pass_header`

```
Syntax:		proxy_pass_header field;
Default:	—
Context:	server, location
```

Permits passing [otherwise disabled](#proxy_hide_header) header fields from a proxied server to a client.

### `proxy_set_header`

```
Syntax:	    proxy_set_header field value;
Default:	proxy_set_header Host proxy_host;
            proxy_set_header Connection close;
Context:	server, location
```

Allows redefining or appending fields to the request header passed to the proxied server. These directives are inherited from the previous configuration level if and only if there are no **proxy_set_header** directives defined on the current level.

### `return`

```
Syntax:		return code [text];
Default:	—
Context:	server, location
```

Stops processing and returns the specified **code** to a client. 
It is possible to specify either a redirect URL (for codes 301, 302, 303, 307, and 308) or the response body text (for other codes).

### `root`

```
Syntax:		root path;
Default:	root .;
Context:	server, location
```

Sets the root directory for requests. For example, with the following configuration
```
location /i/ {
    root /data/w3;
}
```
The **/data/w3/i/top.gif** file will be sent in response to the **“/i/top.gif”** request.

A path to the file is constructed by merely adding a URI to the value of the root directive. If a URI has to be modified, the [alias](#alias) directive should be used.

### `server`

```
Syntax:		server { ... }
Default:	—
Context:	-
```

Sets configuration for a virtual server. There is no clear separation between IP-based (based on the IP address) and name-based (based on the **“Host”** request header field) virtual servers. Instead, the [listen](#listen) directives describe all addresses and ports that should accept connections for the server, and the [server_name](#server_name) directive lists all server names. 

### `server_name`

```
Syntax:		server_name name ...;
Default:	server_name "";
Context:	server
```

Sets names of a virtual server, for example:
```
server {
    server_name example.com;
}
```

If an underscore (_) is used as the server name for a server with an address:port pair, the server will be the default server for that pair.