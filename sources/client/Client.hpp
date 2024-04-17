/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/14 14:58:21 by mgama             #+#    #+#             */
/*   Updated: 2024/04/17 02:14:11 by mgama            ###   ########.fr       */
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

class Client
{
private:
	Server				*_server;
	sockaddr_in			_clientAddr;
	std::string 		_buffer;
	int					_client;
	bool				_headers_received;

	Router				*_current_router;

	time_t				request_time;

	int		processLines(void);

public:
	Client(Server *server, const int client, sockaddr_in clientAddr);
	~Client(void);
	// tmp puclic

	Request		request;
	Response	*response;

	int		process(void);
	bool	timeout(void);
};

#endif /* CLIENT_HPP */