/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/14 14:58:21 by mgama             #+#    #+#             */
/*   Updated: 2024/12/15 01:54:20 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "webserv.hpp"
#include "cluster/Cluster.hpp"
#include "server/Server.hpp"

class Server;
class Request;
class Response;
class Router;
class ProxyWorker;

class Client
{
private:
	Server				*_server;
	int					_client;
	std::string 		_buffer;
	SSL					*_ssl_session;
	bool				_headers_received;

	Router				*_current_router;
	bool				upgraded_to_proxy;

	time_t				request_time;

	int		processLines(void);

public:
	Client(Server *server, const int client, sockaddr_in clientAddr);
	~Client(void);

	Request		request;
	Response	*response;

	int		process(void);
	bool	timeout(void);

	int 	read(char *buffer, size_t buffer_size);
	int 	send(const char *buffer, size_t buffer_size);

	int		getClientFD(void) const;
	SSL		*getSSLSession(void) const;
};

int	serverNameCallback(SSL *ssl, int *ad, void *arg);

#endif /* CLIENT_HPP */