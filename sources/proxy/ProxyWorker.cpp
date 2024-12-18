/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:21 by mgama             #+#    #+#             */
/*   Updated: 2024/12/18 10:22:31 by mgama            ###   ########.fr       */
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

	/**
	 * Résolution DNS de l'adresse du proxy
	 */
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

	/**
	 * La résolution DNS peut retourner plusieurs adresses IP pour un même nom de domaine,
	 * on les teste toutes jusqu'à en trouver une qui fonctionne.
	 */
	for (char **addr = hostent->h_addr_list; *addr != NULL; ++addr) {
		/**
		 * Chaque socket ne peut être utilisé pour qu'un connexion à la fois, il est donc nécessaire
		 * de créer un nouveau socket pour chaque tentative de connexion.
		 */
		this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (this->socket_fd < 0)
		{
			Logger::perror("proxy worker error: socket");
			return (WBS_PROXY_UNAVAILABLE);
		}

		/**
		 * On configure le socket pour qu'il puisse être réutilisé immédiatement après la fermeture
		 */
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

		/**
		 * Tentative de connexion au serveur distant, si la connexion est établie, on sort de la boucle
		 * et on continue le traitement.
		 */
		if (::connect(this->socket_fd, (struct sockaddr *)&this->socket_addr, sizeof(this->socket_addr)) == 0) {
			ss << "Connected to " << inet_ntoa(inAddr);
			Logger::debug(ss.str());
			ss.str("");
			connected = true;
			/**
			 * Mise à jour de l'hôte dans la requête pour qu'elle corresponde à l'hôte du serveur distant
			 */
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

	/**
	 * Si la connexion est sécurisée, on utilise la fonction appropriée pour lire les données
	 */
	if (this->ssl_session)
	{
		valread = SSL_read(this->ssl_session, buffer, buffer_size);
	}
	else
	{
		valread = ::recv(this->socket_fd, buffer, buffer_size, 0);
	}

	return (valread);
}

int	ProxyWorker::send(const char *buffer, size_t buffer_size)
{
	int valread;

	/**
	 * Si la connexion est sécurisée, on utilise la fonction appropriée pour envoyer les données
	 */
	if (this->ssl_session)
	{
		valread = SSL_write(this->ssl_session, buffer, buffer_size);
	}
	else
	{
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

		/**
		 * Version minimale et maximale du protocole TLS à utiliser
		 */
		SSL_CTX_set_min_proto_version(this->ssl_ctx, TLS1_2_VERSION);
		SSL_CTX_set_max_proto_version(this->ssl_ctx, TLS1_3_VERSION);

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

		/**
		 * Ajout du nom de domaine dans les extensions TLS pour le SNI (Server Name Indication)
		 */
		if (SSL_set_tlsext_host_name(this->ssl_session, this->_req.getHost().c_str()) != 1)
		{
			Logger::perror("proxy worker error: SSL_set_tlsext_host_name");
			return (WBS_PROXY_ERROR);
		}

		if (SSL_connect(this->ssl_session) <= 0)
		{
			Logger::perror("proxy worker error: SSL_connect");
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
	 * Ajout du contrôle du contenu de la réponse
	 * 
	 * Pour le moment, le worker renvoie directement le contenu reçu du serveur distant,
	 * sans le traiter.
	 * Il serait judicieux de vérifier le contenu de la réponse, pour potentiellement laisser passer (proxy_pass_header)
	 * ou bloquer (proxy_hide_header) certains en-têtes.
	 */
	if (this->_config.method.length())
	{
		this->_req.setMethod(this->_config.method);
	}
	for (wbs_mapss_t::const_iterator it = this->_config.headers.begin(); it != this->_config.headers.end(); it++)
	{
		this->_req.setHeader((*it).first, (*it).second);
	}

	/**
	 * Préparation des en-têtes de la requête pour le serveur distant
	 */
	std::string data = this->_req.prepareForProxying();
	data.append(WBS_CRLF);
	/**
	 * On ajoute le corps de la requête, si des données ont déjà été envoyées par le client 
	 */
	if (this->_buffer.length() > 0) {
		data.append(this->_buffer);
	}

	/**
	 * Envoi de la requête au serveur distant
	 */
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
	/**
	 * TODO:
	 * 
	 * Gérer le proxy buffering;
	 * Consiste à stocker le contenu reçu depuis le serveur distant puis d'envoyer les données
	 * au client une fois le tampon rempli.
	 */
	fd_set read_fds;
	char buffer[WBS_RECV_SIZE];
	ssize_t bytes_read, bytes_sent;
	size_t total_response_size = 0;

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
			bytes_read = this->_client.read(buffer, WBS_RECV_SIZE);
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

		// Verifier les données provenant du backend
		if (FD_ISSET(backend_fd, &read_fds)) {
			bytes_read = this->read(buffer, WBS_RECV_SIZE);
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

			total_response_size += bytes_sent;
		}
	}

	Server::printProxyResponse(this->_client.method, this->_client.path, getTimestamp() - this->_client.request_time, total_response_size);
	Logger::debug("------------------Proxy task ended-------------------", B_YELLOW);
}
