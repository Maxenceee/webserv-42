/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:20 by mgama             #+#    #+#             */
/*   Updated: 2024/12/24 11:42:56 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROXYWORKER_HPP
# define PROXYWORKER_HPP

#include "webserv.hpp"
#include "client/Client.hpp"
#include "request/Request.hpp"

enum wbs_proxy_code
{
	WBS_PROXY_OK			= 0,
	WBS_PROXY_UNAVAILABLE	= 1,
	WBS_PROXY_ERROR			= 2,
	WBS_PROXY_TIMEOUT		= 3
};

static const char* default_hidden_headers_array[] = {
	"Server",
	"X-Powered-By",
	"Connection",
	"Keep-Alive",
	"Via",
	"X-Forwarded-For",
	"X-Forwarded-Host",
	"X-Forwarded-Proto",
	"Cache-Control",
	"Expires",
	"ETag",
	"Last-Modified"
};

static const std::vector<std::string> default_hidden_headers(
    default_hidden_headers_array,
    default_hidden_headers_array + sizeof(default_hidden_headers_array) / sizeof(default_hidden_headers_array[0])
);

/**
 * FIXME:
 * Ce n'est pas très de definir toute la structure et ses fonctionnalités ici.
 */
struct wbs_proxyworker_client {
	int			fd;
	SSL			*session;
	time_t		request_time;
	std::string	method;
	std::string	path;

	wbs_proxyworker_client(Client *client) {
		this->fd = client->getClientFD();
		this->session = client->getSSLSession();
		this->request_time = client->getRequestTime();
		this->method = client->request.getMethod();
		this->path = client->request.getRawPath();
	}

	int read(char *buffer, size_t size)
	{
		if (this->session)
			return (SSL_read(this->session, buffer, size));
		return (::recv(this->fd, buffer, size, 0));
	}

	int send(char *buffer, size_t size)
	{
		if (this->session)
			return (SSL_write(this->session, buffer, size));
		return (::send(this->fd, buffer, size, 0));
	}

	int send(const char *buffer, size_t size)
	{
		if (this->session)
			return (SSL_write(this->session, buffer, size));
		return (::send(this->fd, buffer, size, 0));
	}
};

class ProxyWorker
{
private:
	struct wbs_proxyworker_client	_client;
	struct wbs_router_proxy			_config;
	int								socket_fd;
	struct sockaddr_in				socket_addr;
	SSL_CTX							*ssl_ctx;
	SSL								*ssl_session;
	const std::string				&_client_buffer;
	Request							&_req;

	int		sendrequest(void);
	int		initSSL(void);

	int		read(char *buffer, size_t buffer_size);
	int		send(const char *buffer, size_t buffer_size);

public:
	ProxyWorker(Client *client, const struct wbs_router_proxy &config, Request &req, const std::string &buffer);
	~ProxyWorker();

	int		connect(void);
	void	work(void);
};

#endif /* PROXYWORKER_HPP */
