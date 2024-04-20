/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/14 14:58:21 by mgama             #+#    #+#             */
/*   Updated: 2024/04/20 14:46:14 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "webserv.hpp"
#include "server/Server.hpp"
#include "routes/Router.hpp"
#include "cluster/Cluster.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "proxy/ProxyClient.hpp"

class ProxyClient;

class Client
{
private:
	Server				*_server;
	sockaddr_in			_clientAddr;
	std::string 		_buffer;
	int					_client;
	bool				_headers_received;

	std::vector<pollfd>				&_poll_fds;
	std::map<int, wbs_pollclient>	&_poll_clients;

	Router				*_current_router;
	ProxyClient			*_proxy;
	bool				is_proxy;

	time_t				request_time;

	int		processLines(void);

public:
	Client(Server *server, const int client, sockaddr_in clientAddr, std::vector<pollfd> &poll_fds, std::map<int, wbs_pollclient> &poll_clients);
	~Client(void);
	// tmp puclic

	Request		request;
	Response	*response;

	int		process(void);
	bool	timeout(void);
};

#endif /* CLIENT_HPP */