/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyClient.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/20 12:51:52 by mgama             #+#    #+#             */
/*   Updated: 2024/04/20 15:00:22 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ProxyClient.hpp"

ProxyClient::ProxyClient(const int client, const struct wbs_router_proxy &config):
	_client(client),
	_config(config),
	socket_fd(-1)
{
}

ProxyClient::~ProxyClient(void)
{
	if (this->socket_fd != -1)
		close(this->socket_fd);
}

int	ProxyClient::connect(void)
{
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
	return (WBS_PROXY_OK);
}

void	ProxyClient::disconnect(void)
{
	std::cout << "proxy client disconnect" << std::endl;
	close(this->socket_fd);
	this->socket_fd = -1;
}

int	ProxyClient::send(const std::string &buffer)
{
	if (::send(this->socket_fd, buffer.c_str(), buffer.size(), 0) < 0)
	{
		perror("send");
		return (WBS_PROXY_ERROR);
	}
	return (WBS_PROXY_OK);
}

int	ProxyClient::send(const Request &request)
{
	std::string res =  request.getMethod() + " " + request.getRawPath() + " HTTP/" + request.getVersion() + WBS_CRLF;
	wbs_mapss_t headers = request.getHeaders();
	for (wbs_mapss_t::iterator it = headers.begin(); it != headers.end(); it++) {
		res += it->first + ": " + it->second + WBS_CRLF;
	}
	res += WBS_CRLF;
	res += request.getBody();

	std::cout << "proxy send: " << res << std::endl;
	if (::send(this->socket_fd, res.c_str(), res.size(), 0) < 0)
	{
		perror("send");
		return (WBS_PROXY_ERROR);
	}
	return (WBS_PROXY_OK);
}

int	ProxyClient::process(void)
{
	if (this->socket_fd == -1)
		return (WBS_POLL_CLIENT_ERROR);

	char buffer[WBS_RECV_SIZE] = {0};

	/**
	 * La fonction recv() sert à lire le contenu d'un descripteur de fichiers, ici
	 * le descripteur du client. À la difference de read(), la fonction recv() est
	 * spécifiquement conçue pour la lecture à partir de socket. Elle offre une meilleure
	 * gestion de la lecture dans un contexte de travail en réseau.
	 */
	int valread = recv(this->socket_fd, buffer, sizeof(buffer), 0);
	if (valread == -1) {
		Logger::error("server error: an error occurred while reading from client");
		return (WBS_POLL_CLIENT_ERROR);
	} else if (valread == 0) {
		Logger::debug("Connection closed by the client");
		return (WBS_POLL_CLIENT_DISCONNECT);
	}

	if (::send(this->_client, buffer, valread, 0) < 0)
	{
		perror("send");
		return (WBS_POLL_CLIENT_ERROR);
	}

	return (WBS_CHUNK_PROCESS_OK);
}

int	ProxyClient::getSocketFD(void) const
{
	return (this->socket_fd);
}