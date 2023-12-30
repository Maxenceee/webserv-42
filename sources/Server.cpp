/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:35:12 by mgama             #+#    #+#             */
/*   Updated: 2023/12/30 18:37:04 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Server.hpp"

Server::Server(int port): port(port)
{
	this->exit = false;
}

Server::~Server(void)
{
}

const int	Server::init(void)
{
	struct sockaddr_in	addr;
	int	option;
	
	std::cout << B_BLUE"Starting server" << std::endl;
	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socket_fd == -1)
	{
		std::cerr << W_PREFIX"error: Could not create socket" << std::endl;
		perror("socket");
		return (W_SOCKET_ERR);
	}
	setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

	// fcntl(this->socket_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(this->port);
	int ret_conn = bind(this->socket_fd, (sockaddr *)&addr, sizeof(addr));
	if (ret_conn == -1)
	{
		std::cerr << W_PREFIX"error: Could not bind port" << std::endl;
		perror("bind");
		return (W_SOCKET_ERR);
	}
	std::cout << B_GREEN"Listening on port " << this->port << std::endl;
	int	error = listen(this->socket_fd, 32);
	if (error != 0)
	{
		std::cerr << W_PREFIX"error: an error occured while listening" << std::endl;
		perror("listen");
		return (W_SOCKET_ERR);
	}
	return (W_NOERR);
}

const int	Server::run(void)
{
	pollfd	fds;
	const int	timeout = (3 * 1000);

	fds.fd = this->socket_fd;
	fds.events = POLLIN;
	do
	{
		if (poll(&fds, 1, timeout) == -1)
		{
			std::cerr << W_PREFIX"error: an error occured while poll'ing" << std::endl;
			perror("poll");
			return (W_SOCKET_ERR);
		}
		sockaddr_in addr = { 0 };
		socklen_t len = sizeof(addr);
		int newClient = accept(this->socket_fd, (sockaddr *)&addr, &len);
		if (newClient > 0)
		{
			printf("request fildes: %d\n", newClient);
			unsigned char *data = readsocket(newClient, NULL, NULL);
			printf("request: %s\n", data);
			close(newClient);
		}
	} while (!this->exit);
	return (W_NOERR);
}

const uint16_t	Server::getPort(void) const
{
	return (this->port);
}

void	Server::setPort(const uint16_t port)
{
	this->port = port;
}