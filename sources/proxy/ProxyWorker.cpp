/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:21 by mgama             #+#    #+#             */
/*   Updated: 2024/11/30 17:12:27 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ProxyWorker.hpp"

ProxyWorker::ProxyWorker(int client, const struct wbs_router_proxy &config, Request &req, const std::string &buffer):
	_client(client),
	_config(config),
	socket_fd(-1),
	_buffer(buffer),
	_req(req)
{
	Logger::debug("New ProxyWorker handling task");
}

ProxyWorker::~ProxyWorker()
{
	Logger::debug("ProxyWorker task completed");
}

int	ProxyWorker::operator()()
{
	if (this->connect() != WBS_PROXY_OK)
	{
		close(this->socket_fd);
		return (WBS_PROXY_UNAVAILABLE);
	}
	for (size_t i = 0; i < this->_config.hidden.size(); i++)
	{
		this->_req.removeHeader(this->_config.hidden[i]);
	}
	for (wbs_mapss_t::const_iterator it = this->_config.headers.begin(); it != this->_config.headers.end(); it++)
	{
		this->_req.addHeader((*it).first, (*it).second);
	}

	std::string data = this->_req.prepareForProxying();
	data.append(WBS_CRLF);
	if (this->_buffer.length() > 0) {
		data.append(this->_buffer);
	}

	if (::send(this->socket_fd, data.c_str(), data.size(), 0) < 0)
	{
		Logger::perror("proxy worker error: send");
		return (WBS_PROXY_ERROR);
	}
	Cluster::pool.enqueueTask(relay_data, this->_client, this->socket_fd);
	Logger::debug("Worker task pushed to pool", RESET);
	if (Logger::_debug)
		std::cout << this->_req << std::endl;
	return (WBS_PROXY_OK);
}

int	ProxyWorker::connect()
{
	int option = 1;

	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socket_fd < 0)
	{
		Logger::perror("proxy worker error: socket");
		return (WBS_PROXY_ERROR);
	}

	if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1) {
		Logger::perror("proxy worker error: setsockopt");
		return (WBS_SOCKET_ERR);
	}

	struct hostent *hostent = gethostbyname(this->_config.host.c_str());
	if (hostent == NULL) {
		if (h_errno == HOST_NOT_FOUND)
			Logger::perror("proxy worker error: host not found");
		else
			Logger::perror("proxy worker error: gethostbyname");
		return (WBS_SOCKET_ERR);
	}

	Logger::debug("DNS resolution for " + this->_config.host + " successful");
	std::stringstream ss;
	ss << "Official name: " << hostent->h_name;
	Logger::debug(ss.str());
	ss.str("");

	bool connected = false;

	bzero(&this->socket_addr, sizeof(this->socket_addr));
	this->socket_addr.sin_family = AF_INET;
	this->socket_addr.sin_port = htons(this->_config.port);

	for (char **addr = hostent->h_addr_list; *addr != NULL; ++addr) {
		struct in_addr inAddr;
		memcpy(&inAddr, *addr, sizeof(struct in_addr));

		ss << "Trying IP Address: " << inet_ntoa(inAddr);
		Logger::debug(ss.str());
		ss.str("");

		memcpy(&this->socket_addr.sin_addr, &inAddr, sizeof(inAddr));

		if (::connect(this->socket_fd, (struct sockaddr *)&this->socket_addr, sizeof(this->socket_addr)) == 0) {
			ss << "Connected to " << inet_ntoa(inAddr);
			Logger::debug(ss.str());
			ss.str("");
			connected = true;
			this->_req.updateHost(this->_config.host);
			break;
		} else {
			Logger::perror("proxy worker error: connect");
		}
	}

	if (!connected) {
		return (WBS_PROXY_ERROR);
	}

	Logger::debug("Proxy tunnel established");

	return (WBS_PROXY_OK);
}

void relay_data(int client_fd, int backend_fd)
{
	fd_set read_fds;
	int max_fd = (client_fd > backend_fd ? client_fd : backend_fd) + 1;
	char buffer[4096];
	ssize_t bytes_read, bytes_sent;

	/**
	 * TODO:
	 * Handle timeout from config
	 */
	while (true) {
		FD_ZERO(&read_fds);
		FD_SET(client_fd, &read_fds);
		FD_SET(backend_fd, &read_fds);

		int activity = select(max_fd, &read_fds, NULL, NULL, NULL);
		if (activity < 0) {
			Logger::perror("proxy worker error: select");
			break;
		}

		// Check if there's data to read from the client
		if (FD_ISSET(client_fd, &read_fds)) {
			bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
			if (bytes_read <= 0) {
				if (bytes_read == 0) {
					// printf("client closed connection\n");
				} else {
					Logger::perror("proxy worker error: recv from client");
				}
				break;
			}

			bytes_sent = send(backend_fd, buffer, bytes_read, 0);
			if (bytes_sent == -1) {
				Logger::perror("proxy worker error: send to backend");
				break;
			}
		}

		// Check if there's data to read from the backend
		if (FD_ISSET(backend_fd, &read_fds)) {
			bytes_read = recv(backend_fd, buffer, sizeof(buffer), 0);
			if (bytes_read <= 0) {
				if (bytes_read == 0) {
					// printf("backend closed connection\n");
				} else {
					Logger::perror("proxy worker error: recv from backend");
				}
				break;
			}

			bytes_sent = send(client_fd, buffer, bytes_read, 0);
			if (bytes_sent == -1) {
				Logger::perror("proxy worker error: send to client");
				break;
			}
		}
	}

	close(client_fd);
	close(backend_fd);
	Logger::debug("------------------Proxy task ended-------------------", B_YELLOW);
}

// enum wbs_proxy_interfacetype
// {
// 	WBS_PROXY_INTERFACE_CLIENT	= 0,
// 	WBS_PROXY_INTERFACE_BACKEND	= 1
// };

// void relay_data(int client_fd, int backend_fd)
// {
// 	pollfd	fds[2];
// 	char	buffer[4096];
// 	ssize_t	bytes_read, bytes_sent;
// 	time_t	current, last_read, last_write;

// 	/**
// 	 * On initialise les descripteurs à surveiller.
// 	 */
// 	fds[WBS_PROXY_INTERFACE_CLIENT] = (pollfd){client_fd, POLLIN, 0};
// 	fds[WBS_PROXY_INTERFACE_BACKEND] = (pollfd){backend_fd, POLLIN, 0};

// 	while (true)
// 	{
// 		if (poll(fds, 2, 100) == -1)
// 		{
// 			if (errno == EINTR) {
// 				break;
// 			}
// 			Logger::perror("proxy worker error: poll");
// 			break;
// 		}

// 		for (int i = 0; i < 2; i++)
// 		{
// 			if (fds[i].revents & POLLHUP)
// 			{
// 				Logger::debug("Connection closed by the client (event POLLHUP)");
// 				return;
// 			}
// 			else if (fds[i].revents & POLLIN)
// 			{
// 				std::cout << "event on client " << fds[i].fd << std::endl;
// 				bytes_read = recv(fds[i].fd, buffer, sizeof(buffer), 0);
// 				std::cout << bytes_read << "bytes_read\n" << buffer << std::endl;
// 				if (bytes_read <= 0)
// 				{
// 					if (bytes_read == 0)
// 					{
// 						Logger::debug("Connection closed by the client");
// 					}
// 					else
// 					{
// 						Logger::perror("proxy worker error: recv");
// 					}
// 					break;
// 				}
// 				if (i == WBS_PROXY_INTERFACE_CLIENT)
// 				{
// 					last_read = getTimestamp();
// 				}

// 				/**
// 				 * Sachant qu'il n'y a que deux descripteurs à surveiller, on peut
// 				 * déterminer l'indice de l'autre par simple soustraction.
// 				 */
// 				std::cout << "send buffer to " << fds[1 - i].fd << std::endl;
// 				bytes_sent = send(fds[1 - i].fd, buffer, bytes_read, 0);
// 				if (bytes_sent == -1)
// 				{
// 					Logger::perror("proxy worker error: send");
// 					break;
// 				}
// 				if (i == WBS_PROXY_INTERFACE_CLIENT)
// 				{
// 					last_write = getTimestamp();
// 				}
// 			}
// 			else
// 			{
// 				/**
// 				 * TODO:
// 				 * Handle timeout from config
// 				 * Current time - last read time > 60s
// 				 */
// 				current = getTimestamp();
// 				if (current - last_read > 60 * 1000 || current - last_write > 60 * 1000)
// 				{
// 					break;
// 				}
// 			}
// 		}
// 	}

// 	close(client_fd);
// 	close(backend_fd);
// 	Logger::debug("------------------Proxy task ended-------------------", B_YELLOW);
// }
