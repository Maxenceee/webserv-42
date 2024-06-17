/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:20 by mgama             #+#    #+#             */
/*   Updated: 2024/06/17 22:57:25 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROXYWORKER_HPP
# define PROXYWORKER_HPP

#include <pthread.h>
#include <sys/select.h>
#include "webserv.hpp"
#include "client/Client.hpp"
#include "routes/Router.hpp"

class Client;

void	relay_data(int client_fd, int backend_fd);

class ProxyWorker
{
private:
	int						_client;
	struct wbs_router_proxy	_config;
	int						socket_fd;
	struct sockaddr_in		socket_addr;
	const std::string		&_buffer;
	// pthread_t				_tid;
	
public:
	ProxyWorker(int client, const struct wbs_router_proxy &config, const std::string &buffer);
	~ProxyWorker();

	int		operator()();	

	int		connect();
};

#endif /* PROXYWORKER_HPP */