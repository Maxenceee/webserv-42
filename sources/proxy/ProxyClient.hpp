/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyClient.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/20 12:52:01 by mgama             #+#    #+#             */
/*   Updated: 2024/04/20 13:38:57 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PROXYCLIENT_HPP
#define PROXYCLIENT_HPP

#include "webserv.hpp"

#include "webserv.hpp"
#include "client/Client.hpp"
#include "routes/Router.hpp"

class Client;

enum wbs_proxy_code
{
	WBS_PROXY_OK		= 0,
	WBS_PROXY_ERROR		= 1,
	WBS_PROXY_TIMEOUT	= 2
};

class ProxyClient
{
private:
	int						_client;
	struct wbs_router_proxy	_config;
	int						socket_fd;
	struct sockaddr_in		socket_addr;

public:
	ProxyClient(const int client, const struct wbs_router_proxy &config);
	~ProxyClient(void);

	int		connect(void);
	void	disconnect(void);
	int		send(const std::string &buffer);
	int		send(const Request &request);
	int		process(void);

	int		getSocketFD(void) const;
};

#endif /* PROXYCLIENT_HPP */