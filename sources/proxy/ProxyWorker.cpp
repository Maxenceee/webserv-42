/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:21 by mgama             #+#    #+#             */
/*   Updated: 2024/12/15 19:42:45 by mgama            ###   ########.fr       */
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
	ssl_ctx(NULL),
	ssl_session(NULL),
	_buffer(buffer),
	_req(req)
{
	Logger::debug("New ProxyWorker handling task");
}

ProxyWorker::~ProxyWorker()
{
	close(this->_client.fd);
	if (this->_client.session)
	{
		SSL_shutdown(this->_client.session);
		SSL_free(this->_client.session);
	}
	if (this->ssl_ctx)
	{
		SSL_CTX_free(this->ssl_ctx);
	}
	if (this->ssl_session)
	{
		SSL_shutdown(this->ssl_session);
		SSL_free(this->ssl_session);
	}
	if (this->socket_fd != -1)
	{
		close(this->socket_fd);
	}
	Logger::debug("ProxyWorker task completed");
}

int	ProxyWorker::connect(void)
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

	if (this->initSSL())
	{
		Logger::error("proxy worker error: could not establish SSL connection");
		return (WBS_PROXY_ERROR);
	}

	return (this->sendrequest());
}

int	ProxyWorker::read(char *buffer, size_t buffer_size)
{
	int valread;

	if (this->ssl_session) {
		/**
		 * La fonction SSL_send() sert à écrire le contenu d'un descripteur de fichier, ici
		 * le descripteur du client. À la différence de write, la fonction SSL_send est
		 * spécifiquement conçue pour écrire dans un socket sécurisé par SSL/TLS.
		 */
		valread = SSL_read(this->ssl_session, buffer, buffer_size);
	} else {
		/**
		 * La fonction send() sert à écrire le contenu d'un descripteur de fichier, ici
		 * le descripteur du client. À la différence de write, la fonction send est
		 * spécifiquement conçue pour écrire dans un socket. Elle offre une meilleure
		 * gestion de l'écriture dans un contexte de travail en réseau.
		 */
		valread = ::recv(this->socket_fd, buffer, buffer_size, 0);
	}

	return (valread);
}

int	ProxyWorker::send(const char *buffer, size_t buffer_size)
{
	int valread;

	if (this->ssl_session) {
		/**
		 * La fonction SSL_write() sert à écrire le contenu d'un descripteur de fichier, ici
		 * le descripteur du client. À la différence de write, la fonction SSL_write est
		 * spécifiquement conçue pour écrire dans un socket sécurisé par SSL/TLS.
		 */
		valread = SSL_write(this->ssl_session, buffer, buffer_size);
	} else {
		/**
		 * La fonction send() sert à écrire le contenu d'un descripteur de fichier, ici
		 * le descripteur du client. À la différence de write, la fonction send est
		 * spécifiquement conçue pour écrire dans un socket. Elle offre une meilleure
		 * gestion de l'écriture dans un contexte de travail en réseau.
		 */
		valread = ::send(this->socket_fd, buffer, buffer_size, 0);
	}

	return (valread);
}

int	ProxyWorker::initSSL(void)
{
	if (this->_config.protocol == "https")
	{
		Logger::debug("Opening SSL session");
		this->ssl_ctx = SSL_CTX_new(TLS_client_method());
		if (!this->ssl_ctx)
		{
			Logger::perror("proxy worker error: SSL_CTX_new");
			return (WBS_PROXY_ERROR);
		}

		this->ssl_session = SSL_new(this->ssl_ctx);
		if (!this->ssl_session)
		{
			Logger::perror("proxy worker error: SSL_new");
			return (WBS_PROXY_ERROR);
		}

		if (SSL_set_fd(this->ssl_session, this->socket_fd) == 0)
		{
			Logger::perror("proxy worker error: SSL_set_fd");
			return (WBS_PROXY_ERROR);
		}

		if (SSL_connect(this->ssl_session) <= 0)
		{
			int ssl_err = SSL_get_error(this->ssl_session, -1);
			switch (ssl_err) {
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
					Logger::debug("SSL_connect: Operation did not complete, retrying...");
					break;  // Réessayer plus tard
				case SSL_ERROR_SYSCALL:
					Logger::perror("proxy worker error: SSL_connect (SSL_ERROR_SYSCALL)");
					break;
				case SSL_ERROR_SSL:
					Logger::perror("proxy worker error: SSL_connect (SSL_ERROR_SSL)");
					ERR_print_errors_fp(stderr);  // Affiche les erreurs internes d'OpenSSL
					break;
				default:
					Logger::perror("proxy worker error: SSL_connect (Unknown error)");
					break;
			}
			return (WBS_PROXY_ERROR);
		}
		Logger::debug("SSL session established");
	}
	return (WBS_PROXY_OK);
}

int	ProxyWorker::sendrequest(void)
{
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
		this->_req.setHeader((*it).first, (*it).second);
	}

	std::string data = this->_req.prepareForProxying();
	data.append(WBS_CRLF);
	if (this->_buffer.length() > 0) {
		data.append(this->_buffer);
	}

	if (this->send(data.c_str(), data.size()) < 0)
	{
		Logger::perror("proxy worker error: send");
		return (WBS_PROXY_ERROR);
	}
	if (Logger::isDebug())
		std::cout << this->_req << std::endl;
	return (WBS_PROXY_OK);
}

void	ProxyWorker::work(void)
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

			bytes_sent = this->send(buffer, bytes_read);
			if (bytes_sent == -1)
			{
				Logger::perror("proxy worker error: send to backend");
				break;
			}
		}

		// Check if there's data to read from the backend
		if (FD_ISSET(backend_fd, &read_fds)) {
			bytes_read = this->read(buffer, sizeof(buffer));
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
