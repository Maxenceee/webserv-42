/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:35:12 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 13:09:36 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Server.hpp"
#include "response/Response.hpp"
#include "request/Request.hpp"

Server::Server(int port): port(port)
{
	this->exit = false;
}

Server::~Server(void)
{
}

const int	Server::init(void)
{
	int	option = 1;
	
	FD_ZERO(&this->_fd_set);
	std::cout << B_BLUE"Starting server" << RESET << std::endl;
	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	FD_SET(this->socket_fd, &this->_fd_set);
	if (this->socket_fd == -1)
	{
		std::cerr << W_PREFIX"error: Could not create socket" << std::endl;
		perror("socket");
		return (W_SOCKET_ERR);
	}
	if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1) {
		perror("setsockopt");
		return (W_SOCKET_ERR);
	}

	// if (fcntl(this->socket_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1) {
	// 	perror("fcntl");
	// 	return (W_SOCKET_ERR);
	// }

	memset(&this->socket_addr, 0, sizeof(this->socket_addr));
	this->socket_addr.sin_family = AF_INET;
	this->socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->socket_addr.sin_port = htons(this->port);
	int ret_conn = bind(this->socket_fd, (sockaddr *)&this->socket_addr, sizeof(this->socket_addr));
	if (ret_conn == -1)
	{
		std::cerr << W_PREFIX"error: Could not bind port" << std::endl;
		perror("bind");
		return (W_SOCKET_ERR);
	}
	return (W_NOERR);
}

const int	Server::start(void)
{
	pollfd	fds;
	const int	timeout = (3 * 1000);

	std::cout << B_GREEN"Listening on port " << this->port << RESET << std::endl;
	int	error = listen(this->socket_fd, 32);
	if (error == -1)
	{
		std::cerr << W_PREFIX"error: an error occured while listening" << std::endl;
		perror("listen");
		return (W_SOCKET_ERR);
	}

	do
	{
		fds.fd = this->socket_fd;
		fds.events = POLLIN;

		if (poll(&fds, 1, timeout) == -1)
		{
			std::cerr << W_PREFIX"error: an error occured while poll'ing" << std::endl;
			perror("poll");
			return (W_SOCKET_ERR);
		}
		if (fds.revents & POLLIN) {
			socklen_t len = sizeof(this->socket_addr);
			int newClient = accept(this->socket_fd, (sockaddr *)&this->socket_addr, &len);
			if (newClient == -1)
			{
				perror("accept");
    			continue;
			}
			// // char buffer[100];
			// // int read_bytes = read(newClient, buffer, 100);
			// std::cout << "The message was: " << buffer;
			// std::string response = "Good talking to you\n";
			// send(newClient, response.c_str(), response.size(), 0);
			char buffer[RECV_SIZE] = {0};
			// char *hello = "HTTP/1.1 200 OK\nContent-Length: 12\nContent-Type: text/plain\n\nHello world!\n";

			int valread = recv(newClient, buffer, sizeof(buffer), 0);
			if (valread > 0) {
				printf("Received %d bytes: \n%s\n", valread, buffer);

				Request	request = Request(std::string(buffer));
				std::cout << request << std::endl;

				Response response = Response(newClient, request.getVersion(), request.getSatus());
				response.send("hello world!\n").end();
				printf(B_YELLOW"------------------Response sent-------------------%s\n", RESET);
			} else if (valread == 0) {
				printf("Connection closed by the client\n");
			} else {
				if (errno == EAGAIN || errno == EWOULDBLOCK) {
					printf("not ready to read\n");
				} else {
					perror("recv");
				}
			}
			close(newClient);
		}
	} while (!this->exit);
	close(this->socket_fd);
	return (W_NOERR);
}

void	Server::kill(void)
{
	this->exit = true;
	close(this->socket_fd);
}

const uint16_t	Server::getPort(void) const
{
	return (this->port);
}

void	Server::setPort(const uint16_t port)
{
	this->port = port;
}