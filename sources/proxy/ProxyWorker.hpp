/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:20 by mgama             #+#    #+#             */
/*   Updated: 2024/04/17 20:53:22 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROXYWORKER_HPP
# define PROXYWORKER_HPP

#include "webserv.hpp"
#include "client/Client.hpp"
#include "routes/Router.hpp"

class Client;

enum wbs_proxy_code
{
	WBS_PROXY_OK = 0,
	WBS_PROXY_ERROR = 1,
	WBS_PROXY_TIMEOUT = 2
};

class ProxyWorker
{
private:
	int						_client;
	struct wbs_router_proxy	_config;
	int						socket_fd;
	struct sockaddr_in		socket_addr;
	pid_t					_pid;
	
public:
	ProxyWorker(int client, const struct wbs_router_proxy &config);
	~ProxyWorker();

	int		connect(const std::string &buffer);
};

#endif /* PROXYWORKER_HPP */