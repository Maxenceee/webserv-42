/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:20 by mgama             #+#    #+#             */
/*   Updated: 2024/06/19 11:30:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROXYWORKER_HPP
# define PROXYWORKER_HPP

#include "webserv.hpp"
#include "client/Client.hpp"
#include "routes/Router.hpp"
#include "request/Request.hpp"

class Client;

void	relay_data(int client_fd, int backend_fd);

enum wbs_proxy_code
{
	WBS_PROXY_OK		= 0,
	WBS_PROXY_ERROR		= 1,
	WBS_PROXY_TIMEOUT	= 2
};

class ProxyWorker
{
private:
	int						_client;
	struct wbs_router_proxy	_config;
	int						socket_fd;
	struct sockaddr_in		socket_addr;
	const std::string		&_buffer;
	Request					&_req;
	// pthread_t				_tid;
	
public:
	ProxyWorker(int client, const struct wbs_router_proxy &config, Request &req, const std::string &buffer);
	~ProxyWorker();

	int		operator()();	

	int		connect();
};

#endif /* PROXYWORKER_HPP */