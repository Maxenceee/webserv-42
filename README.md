# Webserv

My Webserv project for the 42 School cursus, a [`Nginx`](https://nginx.org) like web server made in c++98. This project is much more complete and complex than what is required by the subject to be validated at 125. It includes many Nginx directives and advanced features such as proxies. See the [Features](#features) section below.

Final grade : `125/100` & Outstanding Project

## Getting started

The server can be used in the classic way, or can be daemonized via launchd (OS X) or systemd (Linux).

To start the server:

``` sh
$> ./webserv [config_file]
```

Use `-d` or `--debug` parameter to enable debug mode.

To daemonize the server:

``` sh
$>  ./setup_daemon.bash
```

In this case you need to place your config file in `/etc/webserv` with the `.conf` extension.

Log file will be `/var/log/webserv.log` and `/var/log/webserv.err`.

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