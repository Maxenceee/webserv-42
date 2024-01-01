/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:35:12 by mgama             #+#    #+#             */
/*   Updated: 2023/12/31 18:31:36 by mgama            ###   ########.fr       */
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
	int	option;
	
	std::cout << B_BLUE"Starting server"RESET << std::endl;
	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socket_fd == -1)
	{
		std::cerr << W_PREFIX"error: Could not create socket" << std::endl;
		perror("socket");
		return (W_SOCKET_ERR);
	}
	setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

	// fcntl(this->socket_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);

	this->socket_addr.sin_family = AF_INET;
	this->socket_addr.sin_addr.s_addr = INADDR_ANY;
	this->socket_addr.sin_port = htons(this->port);
	int ret_conn = bind(this->socket_fd, (sockaddr *)&this->socket_addr, sizeof(this->socket_addr));
	if (ret_conn == -1)
	{
		std::cerr << W_PREFIX"error: Could not bind port" << std::endl;
		perror("bind");
		return (W_SOCKET_ERR);
	}
	std::cout << B_GREEN"Listening on port " << this->port << RESET << std::endl;
	int	error = listen(this->socket_fd, 32);
	if (error != 0)
	{
		std::cerr << W_PREFIX"error: an error occured while listening" << std::endl;
		perror("listen");
		return (W_SOCKET_ERR);
	}
	// Create a socket (IPv4, TCP)
	// int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	// if (sockfd == -1) {
	// 	std::cout << "Failed to create socket. errno: " << errno << std::endl;
	// 	return (W_SOCKET_ERR);
	// }

	// // Listen to port 9999 on any address
	// sockaddr_in sockaddr;
	// sockaddr.sin_family = AF_INET;
	// sockaddr.sin_addr.s_addr = INADDR_ANY;
	// sockaddr.sin_port = htons(this->port); // htons is necessary to convert a number to
	// 								// network byte order
	// if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
	// 	std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
	// 	return (W_SOCKET_ERR);
	// }

	// // Start listening. Hold at most 10 connections in the queue
	// if (listen(sockfd, 10) < 0) {
	// 	std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
	// 	return (W_SOCKET_ERR);
	// }

	// // Grab a connection from the queue
	// auto addrlen = sizeof(sockaddr);
	// int connection = accept(sockfd, (struct sockaddr*)&sockaddr, (socklen_t*)&addrlen);
	// if (connection < 0) {
	// 	std::cout << "Failed to grab connection. errno: " << errno << std::endl;
	// 	return (W_SOCKET_ERR);
	// }

	// // Read from the connection
	// char buffer[100];
	// auto bytesRead = read(connection, buffer, 100);
	// std::cout << "The message was: " << buffer;

	// // Send a message to the connection
	// std::string response = "Good talking to you\n";
	// send(connection, response.c_str(), response.size(), 0);

	// // Close the connections
	// close(connection);
	// close(sockfd);
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
		socklen_t len = sizeof(this->socket_addr);
		int newClient = accept(this->socket_fd, (sockaddr *)&this->socket_addr, &len);
		if (newClient > 0)
		{
			printf("request fildes: %d\n", newClient);
			unsigned char *buffer = readsocket(newClient, NULL, NULL);
			// char buffer[100];
			// int read_bytes = read(newClient, buffer, 100);
			std::cout << "The message was: " << buffer;
			std::string response = "Good talking to you\n";
			send(newClient, response.c_str(), response.size(), 0);
			close(newClient);
		}
	} while (!this->exit);
	close(this->socket_fd);
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