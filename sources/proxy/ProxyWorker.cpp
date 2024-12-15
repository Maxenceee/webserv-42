/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:21 by mgama             #+#    #+#             */
/*   Updated: 2024/12/15 14:09:17 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ProxyWorker.hpp"

/**
 * TODO:
 * Gérer le fait d'avoir un URI dans la config du proxy
 * ex: 
 * proxy_pass http://localhost:3000/api;
 */

ProxyWorker::ProxyWorker(Client *client, const struct wbs_router_proxy &config, Request &req, const std::string &buffer):
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
	close(this->_client.fd);
	if (this->_client.session) {
		SSL_shutdown(this->_client.session);
		SSL_free(this->_client.session);
	}
	close(this->socket_fd);
	Logger::debug("ProxyWorker task completed");
}

int	ProxyWorker::connect()
{
	int option = 1;

	struct hostent *hostent = gethostbyname(this->_config.host.c_str());
	if (hostent == NULL) {
		if (h_errno == HOST_NOT_FOUND)
			Logger::perror("proxy worker error: host not found");
		else
			Logger::perror("proxy worker error: gethostbyname");
		return (WBS_PROXY_UNAVAILABLE);
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
		this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (this->socket_fd < 0)
		{
			Logger::perror("proxy worker error: socket");
			return (WBS_PROXY_UNAVAILABLE);
		}

		if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1) {
			Logger::perror("proxy worker error: setsockopt");
			return (WBS_PROXY_UNAVAILABLE);
		}
		struct in_addr inAddr;
		memcpy(&inAddr, *addr, sizeof(struct in_addr));

		ss << "Trying IP Address: " << inet_ntoa(inAddr) << " on port " << this->_config.port;
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
			close(this->socket_fd);
		}
	}

	if (!connected) {
		return (WBS_PROXY_ERROR);
	}

	Logger::debug("Proxy tunnel established");

	return (this->sendrequest());
}

int	ProxyWorker::sendrequest()
{
	if (this->socket_fd == -1)
	{
		throw std::runtime_error("ProxyWorker: worker not initialized or connection failed");
	}
	/**
	 * TODO:
	 * Support forward headers
	 * See: Router::wbs_router_proxy::forwared
	 */
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
	if (Logger::isDebug())
		std::cout << this->_req << std::endl;
	return (WBS_PROXY_OK);
}

void	ProxyWorker::work()
{
	fd_set read_fds;
	char buffer[4096];
	ssize_t bytes_read, bytes_sent;;

	int client_fd = this->_client.fd;
	int backend_fd = this->socket_fd;

	int max_fd = (client_fd > backend_fd ? client_fd : backend_fd) + 1;

	struct timeval timeout;
	timeout.tv_sec = 60;
	timeout.tv_usec = 0;

	while (true) {
		FD_ZERO(&read_fds);
		FD_SET(client_fd, &read_fds);
		FD_SET(backend_fd, &read_fds);

		int activity = select(max_fd, &read_fds, NULL, NULL, &timeout);
		if (activity == -1)
		{
			Logger::perror("proxy worker error: select");
			break;
		}
		else if (activity == 0)
		{
			Logger::debug("proxy worker: timeout reached, closing connection");
			break;
		}

		// Vérifier les données provenant du client
		if (FD_ISSET(client_fd, &read_fds)) {
			bytes_read = this->_client.read(buffer, sizeof(buffer));
			if (bytes_read == 0)
			{
				Logger::debug("proxy worker: client closed the connection");
				break;
			}
			else if (bytes_read == -1)
			{
				Logger::perror("proxy worker error: recv from client");
				break;
			}

			bytes_sent = send(backend_fd, buffer, bytes_read, 0);
			if (bytes_sent == -1)
			{
				Logger::perror("proxy worker error: send to backend");
				break;
			}
		}

		// Check if there's data to read from the backend
		if (FD_ISSET(backend_fd, &read_fds)) {
			bytes_read = recv(backend_fd, buffer, sizeof(buffer), 0);
			if (bytes_read == 0)
			{
				Logger::debug("proxy worker: backend closed the connection");
				break;
			}
			else if (bytes_read == -1)
			{
				Logger::perror("proxy worker error: recv from client");
				break;
			}

			bytes_sent = this->_client.send(buffer, bytes_read);
			if (bytes_sent == -1)
			{
				Logger::perror("proxy worker error: send to client");
				break;
			}
		}
	}

	Server::printProxyResponse(this->_client.method, this->_client.path, getTimestamp() - this->_client.request_time);
	Logger::debug("------------------Proxy task ended-------------------", B_YELLOW);
}
