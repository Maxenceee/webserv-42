/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:21 by mgama             #+#    #+#             */
/*   Updated: 2024/04/17 15:11:25 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ProxyWorker.hpp"

ProxyWorker::ProxyWorker(int client, const struct wbs_router_proxy &config): _client(client), _config(config), socket_fd(-1)
{
	
}

ProxyWorker::~ProxyWorker()
{
	
}

int	ProxyWorker::connect()
{
	switch (fork())
	{
	case -1:
		perror("fork");
		return (WBS_PROXY_ERROR);
	case 0:
		close(this->_client);
		return (WBS_PROXY_OK);
	}
	int option = 1;

	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socket_fd < 0)
	{
		perror("socket");
		return (WBS_PROXY_ERROR);
	}

	if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1) {
		perror("setsockopt");
		return (WBS_SOCKET_ERR);
	}

	struct sockaddr_in addr;
	bzero(&this->socket_addr, sizeof(this->socket_addr));
	this->socket_addr.sin_family = AF_INET;
	this->socket_addr.sin_port = htons(this->_config.port);
	this->socket_addr.sin_addr.s_addr = htonl(setIPAddress(this->_config.host));

	if (::connect(this->socket_fd, (struct sockaddr *)&this->socket_addr, sizeof(this->socket_addr)) < 0)
	{
		perror("connect");
		return (WBS_PROXY_ERROR);
	}
}
