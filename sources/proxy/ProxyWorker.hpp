/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:20 by mgama             #+#    #+#             */
/*   Updated: 2024/12/14 20:13:31 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROXYWORKER_HPP
# define PROXYWORKER_HPP

#include "webserv.hpp"
#include "client/Client.hpp"
#include "routes/Router.hpp"
#include "request/Request.hpp"

class Client;

void	relay_data(wbs_threadpool_client client, wbs_threadpool_backend backend);

enum wbs_proxy_code
{
	WBS_PROXY_OK			= 0,
	WBS_PROXY_UNAVAILABLE	= 1,
	WBS_PROXY_ERROR			= 2,
	WBS_PROXY_TIMEOUT		= 3
};

struct wbs_proxyworker_client {
	int		fd;
	SSL		*session;

	wbs_proxyworker_client(Client *client) {
		this->fd = client->getClientFD();
		this->session = client->getSSLSession();
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
};

class ProxyWorker
{
private:
	struct wbs_proxyworker_client	*_client;
	struct wbs_router_proxy			_config;
	int								socket_fd;
	struct sockaddr_in				socket_addr;
	const std::string				&_buffer;
	Request							&_req;
	// pthread_t				_tid;
	
public:
	ProxyWorker(Client *client, const struct wbs_router_proxy &config, Request &req, const std::string &buffer);
	~ProxyWorker();

	int		operator()();	

	int		connect();
};

#endif /* PROXYWORKER_HPP */