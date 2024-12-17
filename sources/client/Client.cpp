/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:35:12 by mgama             #+#    #+#             */
/*   Updated: 2024/12/17 12:32:14 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "cluster/Cluster.hpp"
#include "proxy/ProxyWorker.hpp"
#include "base64.hpp"
#include "websocket/websocket.hpp"

Client::Client(Server *server, const int client, sockaddr_in clientAddr):
	_server(server),
	_client(client),
	_ssl_session(NULL),
	_headers_received(false),
	_current_router(NULL),
	upgraded_to_proxy(false),
	request_time(getTimestamp()),
	request(client, clientAddr, server->hasSSL()),
	response(NULL)
{
	if (server->hasSSL())
	{
		/**
		 * Création d'un contexte SSL générique pour le serveur.
		 * Ce contexte est utilisé pour créer une nouvelle session SSL pour chaque connexion,
		 * ce contexte est temporaire et ne sert qu'à créer une session SSL pour le client.
		 * Il est remplacé par le bon contexte SSL lors de la réception du SNI.
		 */
		SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_server_method());
		if (!ssl_ctx)
		{
			throw std::runtime_error("Failed to create SSL_CTX");
		}

		/**
		 * Ajout de la fonction de rappel pour le nom du serveur (SNI).
		 */
		SSL_CTX_set_tlsext_servername_callback(ssl_ctx, &serverNameCallback);
		/**
		 * On passe le serveur en tant qu'argument pour la fonction de rappel, afin de pouvoir
		 * récupérer les configurations du serveur en fonction du nom du serveur.
		 */
		SSL_CTX_set_tlsext_servername_arg(ssl_ctx, server);

		/**
		 * Création de la session SSL pour le client.
		 */
		this->_ssl_session = SSL_new(ssl_ctx);
		if (!this->_ssl_session)
		{
			SSL_CTX_free(ssl_ctx);
			throw std::runtime_error("Failed to create SSL session");
		}

		/**
		 * Assignation du descripteur de fichier du client à la session SSL.
		 */
		SSL_set_fd(this->_ssl_session, client);

		/**
		 * Acceptation de la connexion SSL.
		 */
		if (SSL_accept(this->_ssl_session) <= 0)
		{
			/**
			 * Si l'erreur est due a une interruption de la connexion, on ignore l'erreur, car
			 * cela signifie que le client a fermé la connexion, cas qui est géré par la suite.
			 */
			if (!(errno == EPIPE || errno == ECONNRESET))
			{
			
				SSL_CTX_free(ssl_ctx);
				SSL_free(this->_ssl_session);
				Logger::perror("Failed to accept SSL connection");
				throw std::runtime_error("Failed to accept SSL connection");
			}
		}
		SSL_CTX_free(ssl_ctx);
	}
}

Client::~Client(void)
{
	if (this->response && this->upgraded_to_proxy)
	{
		/**
		 * Lorsque le client passe en mode proxy, il passe le relais au ProxyWorker
		 * et doit être détruit sans fermer la connexion avec le client.
		 * Dans ce cas, on annule la réponse et on supprime le pointeur.
		 */
		this->response->cancel();
		delete this->response;
		/**
		 * On ne ferme pas la connexion avec le client car désormais c'est le ProxyWorker qui s'occupe de la communication.
		 */
		Logger::debug("------------------Client upgraded to proxy-------------------", B_YELLOW);
		return ;
	}
	if (this->response && this->response->isUpgraded())
	{
		/**
		 * Dans le cas où le client a demandé une mise à niveau vers WebSocket, on ne ferme pas la connexion et
		 * on affiche les trames WebSocket du client.
		 */
		Server::printResponse(this->request, *this->response, getTimestamp() - this->request_time);
		delete this->response;
		close(this->_client);
		Logger::debug("------------------Client WebSocket closed-------------------", B_YELLOW);
		return ;
	}
	if (this->response)
	{
		if (Logger::isDebug())
			std::cout << *this->response << std::endl;
		Server::printResponse(this->request, *this->response, getTimestamp() - this->request_time);
		delete this->response;
	}
	if (this->_ssl_session)
	{
		SSL_shutdown(this->_ssl_session);
		SSL_free(this->_ssl_session);
	}
	close(this->_client);
	Logger::debug("------------------Client connection closed-------------------", B_YELLOW);
}

int	Client::process(void)
{
	char buffer[WBS_RECV_SIZE] = {0};

	/**
	 * La fonction recv() sert à lire le contenu d'un descripteur de fichier, ici
	 * le descripteur du client. À la différence de read(), la fonction recv() est
	 * spécifiquement conçue pour la lecture à partir de sockets. Elle offre une meilleure
	 * gestion de la lecture dans un contexte de travail en réseau.
	 */
	int valread = this->read(buffer, WBS_RECV_SIZE);
	if (valread == -1)
	{
		Logger::error("server error: an error occurred while reading from client");
		return (WBS_POLL_CLIENT_ERROR);
	}
	else if (valread == 0)
	{
		Logger::debug("Connection closed by the client");
		return (WBS_POLL_CLIENT_DISCONNECT);
	}

	this->_buffer.append(buffer, valread);

	if (this->processLines())
	{
		// La sauvegarde du temps est nécessaire pour le calcul de la durée de la requête
		this->request_time = getTimestamp();
		return (WBS_POLL_CLIENT_CLOSED);
	}

	if (this->response && this->response->isUpgraded())
	{
		/**
		 * Dans le cas où le client a demandé une mise à niveau vers WebSocket, on ne ferme pas la connexion et
		 * on affiche les trames WebSocket du client.
		 */
		std::string payload = decodeWebSocketFrame(this->_buffer);

		if (payload == "ping")
			this->response->sendFrame("pong");
		if (payload == "oui")
			this->response->sendFrame("non");

		this->_buffer.clear();
		return (WBS_POLL_CLIENT_OK);
	}

	if (this->request.processFinished())
	{
		if (this->_current_router->isProxy())
		{
			return (WBS_POLL_CLIENT_CLOSED);
		}
		else
		{
			// La sauvegarde du temps est necessaire pour le calcul de la durée de la requête
			this->request_time = getTimestamp();
			/**
			 * Une fois que la requête est complètement analysée, on peut effectuer le routage.
			 */
			if (Logger::isDebug())
				std::cout << this->request << std::endl;
			this->_current_router->route(this->request, *this->response);
			/**
			 * Pour le moment, le serveur ne gère pas le keep-alive (en-tête connection).
			 * On indique donc au cluster qu'il faut fermer la connexion.
			 */
		}
		return (WBS_POLL_CLIENT_CLOSED);
	}
	return (WBS_POLL_CLIENT_OK);
}

int	Client::processLines(void)
{
	/**
	 * Dans le cas où le client a demandé une mise à niveau vers WebSocket, évite de traiter la
	 * donnée.
	 */
	if (this->response && this->response->isUpgraded())
	{
		return (WBS_NOERR);
	}

	size_t pos;
	while (!this->request.headersReceived() && (pos = this->_buffer.find(WBS_CRLF)) != std::string::npos)
	{
		std::string line = this->_buffer.substr(0, pos); // Extraire une ligne complète du buffer
		this->_buffer.erase(0, pos + 2); // Supprimer la ligne traitée du buffer (incluant \r\n)

		if (this->request.processLine(line))
		{
			// En cas d'erreur de parsing, on envoie une réponse d'erreur
			this->response->status(400).end();
			return (WBS_ERR);
		}

		if (this->response == NULL)
		{
			this->response = new Response(*this, request);
			if (!this->response->canSend())
			{
				return (WBS_ERR);
			}
		}

		/**
		 * Extraction du routeur correspondant à la requête dès l'arrivée de l'en-tête `Host`.
		 */
		if (this->_current_router == NULL && this->request.hasHeader("Host"))
		{
			/**
			 * La fonction Server::eval() retourne un pointeur vers le routeur correspondant à la requête
			 * ou le routeur par défaut du serveur si aucun routeur correspondant n'a été trouvé, cette fonction
			 * n'est donc jamais censée retourner NULL, mais on sécurise tout de même.
			 */
			this->_current_router = this->_server->eval(this->request, *this->response);
			if (this->_current_router == NULL)
			{
				this->response->status(404).sendDefault().end();
				return (WBS_ERR);
			}
			Logger::debug("Router matched: " + this->_current_router->getLocation().path);
		}

		/**
		 * On vérifie si les en-têtes ont été reçus. Auquel cas, on peut traiter les cas de requêtes
		 * ne nécessitant pas de corps.
		 */
		if (!this->_headers_received && this->request.headersReceived())
		{
			this->_headers_received = true;

			/**
			 * Si lors de l'évaluation de la requête, il y a eu une erreur, le code de statut de la réponse
			 * est alors différent de 200, on envoie donc une réponse d'erreur.
			 */
			if (this->response->getStatus() != 200)
			{
				this->_current_router->sendResponse(*this->response);
				return (WBS_ERR);
			}
			
			if (this->request.hasHeader("Connection") && this->request.getHeader("Connection") == "Upgrade")
			{
				/**
				 *
				 * INFO:
				 * Experimentation du protocole WebSocket.
				 * 
				 * @see https://www.rfc-editor.org/rfc/rfc7118.html
				 * 
				 */
				std::cout << this->request << std::endl;

				if (!this->request.hasHeader("Upgrade") || this->request.getHeader("Upgrade") != "websocket")
				{
					this->response->status(426).sendDefault().end();
					return (WBS_ERR);
				}
				if (!this->request.hasHeader("Sec-Websocket-Key"))
				{
					Logger::error("WebSocket error: missing Sec-Websocket-Key header");
					this->response->status(400).sendDefault().end();
					return (WBS_ERR);
				}

				/**
				 * Le protolole WebSocket exige qu'une clé soit envoyée par le client.
				 */
				std::string key = this->request.getHeader("Sec-Websocket-Key");

				/**
				 * le client a demandé une mise à niveau, nous devons le gérer et renvoyer la réponse appropriée.
				 */
				this->response->setHeader("Connection", "upgrade");
				this->response->setHeader("Upgrade", "websocket");
				this->response->setHeader("Sec-WebSocket-Accept", calculateSecWebSocketAccept(key));

				/**
				 * INFO:
				 * Pour supporter le protocole WebSocket, il faudrait créer un état supplémentaire, qui empêcherait la
				 * fermeture de la connexion avec le client une fois la requête traitée.
				 * Dans le cas d'un serveur statique, une telle implémentation n'a aucun intérêt puisque le serveur ne
				 * peut pas traiter les requêtes WebSocket.
				 */
				this->response->status(101).upgrade().end();
				return (WBS_NOERR);
			}

			/**
			 * TODO:
			 * 
			 * Gérer le proxy buffering;
			 * consiste à stocker le contenu reçu depuis le client jusqu'à ce que la taille du buffer soit atteinte 
			 * puis d'envoyer tout le buffer au serveur distant.
			 * Note: pas sûr de le faire.
			 */
			if (this->_current_router->isProxy())
			{
				/**
				 * Si le routeur est un proxy, on crée un ProxyWorker qui va se charger de la communication
				 * avec le serveur distant.
				 */
				ProxyWorker *worker = new ProxyWorker(this, this->_current_router->getProxyConfig(), this->request, this->_buffer);
				switch (worker->connect())
				{
				/**
				 * Si une erreur s'est produite lors de la création du ProxyWorker,
				 * on envoie une réponse d'erreur 503 (Service Unavailable) informant le client que la connexion
				 * avec le serveur distant n'a pas pu être établie.
				 */
				case WBS_PROXY_UNAVAILABLE:
					this->response->status(503).sendDefault().end();
					delete worker;
					// La session SSL est détruite par le ProxyWorker donc on la met à NULL pour éviter de la détruire deux fois
					this->_ssl_session = NULL;
					return (WBS_ERR);
				/**
				 * Ou le code 502 (Bad Gateway) informant qu'il y a eu une erreur lors de la communication
				 * avec le serveur distant.
				 */
				case WBS_PROXY_ERROR:
					this->response->status(502).sendDefault().end();
					delete worker;
					this->_ssl_session = NULL;
					return (WBS_ERR);
				case WBS_PROXY_OK:
					this->upgraded_to_proxy = true;
					break;
				}
				Cluster::pool.enqueueWorker(worker);
				Logger::debug("Worker task pushed to pool", RESET);
				return (WBS_POLL_CLIENT_CLOSED);
			}
		}
	}

	if (!this->_buffer.empty() && this->request.headersReceived())
	{
		/**
		 * Si le client envoie un corps de requête alors que la requête n'en attend pas,
		 * une réponse d'erreur est envoyée.
		 */
		if (this->request.bodyReceived() && !this->request.hasContentLength())
		{
			this->response->status(411).sendDefault().end();
			return (WBS_ERR);
		}

		if (request.processLine(this->_buffer))
		{
			// En cas d'erreur de parsing, on envoie une réponse d'erreur
			this->response->status(400).end();
			return (WBS_ERR);
		}
		this->_buffer.clear();
		return (WBS_NOERR);
	}
	return (WBS_NOERR);
}

int	Client::read(char *buffer, size_t buffer_size)
{
	int valread;

	if (this->_ssl_session)
	{
		/**
		 * La fonction SSL_read() sert à lire le contenu d'un descripteur de fichier, ici
		 * le descripteur du client. À la différence de recv(), la fonction SSL_read() est
		 * spécifiquement conçue pour la lecture à partir de sockets sécurisés par SSL/TLS.
		 */
		valread = SSL_read(this->_ssl_session, buffer, buffer_size);
	}
	else 
	{
		/**
		 * La fonction recv() sert à lire le contenu d'un descripteur de fichier, ici
		 * le descripteur du client. À la différence de read(), la fonction recv() est
		 * spécifiquement conçue pour la lecture à partir de sockets. Elle offre une meilleure
		 * gestion de la lecture dans un contexte de travail en réseau.
		 */
		valread = ::recv(this->_client, buffer, buffer_size, 0);
	}

	return (valread);
}

int	Client::send(const char *buffer, size_t buffer_size)
{
	int valread;

	if (this->_ssl_session)
	{
		/**
		 * La fonction SSL_write() sert à écrire le contenu d'un descripteur de fichier, ici
		 * le descripteur du client. À la différence de write, la fonction SSL_write est
		 * spécifiquement conçue pour écrire dans un socket sécurisé par SSL/TLS.
		 */
		valread = SSL_write(this->_ssl_session, buffer, buffer_size);
	}
	else
	{
		/**
		 * La fonction send() sert à écrire le contenu d'un descripteur de fichier, ici
		 * le descripteur du client. À la différence de write, la fonction send est
		 * spécifiquement conçue pour écrire dans un socket. Elle offre une meilleure
		 * gestion de l'écriture dans un contexte de travail en réseau.
		 */
		valread = ::send(this->_client, buffer, buffer_size, 0);
	}

	return (valread);
}

int	Client::getClientFD(void) const
{
	return (this->_client);
}

SSL	*Client::getSSLSession(void) const
{
	return (this->_ssl_session);
}

time_t	Client::getRequestTime(void) const
{
	return (this->request_time);
}

int	serverNameCallback(SSL *ssl, int *ad, void *arg)
{
	(void)ad;
	const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
	
	if (servername == NULL)
	{
		Logger::error("SSL error: no server name received");
		return (SSL_TLSEXT_ERR_ALERT_FATAL);
	}

	ServerConfig *config = ((Server *)arg)->getConfig(servername);
	/**
	 * La fonction SSL_set_SSL_CTX() permet de définir le contexte SSL à utiliser pour la session SSL.
	 */
	if (SSL_set_SSL_CTX(ssl, config->getSSLCTX()) == 0)
	{
		Logger::error("SSL error: could not set SSL context");
		return (SSL_TLSEXT_ERR_ALERT_FATAL);
	}
	
	return (SSL_TLSEXT_ERR_OK);
}

bool	Client::timeout(void)
{
	/**
	 * TODO:
	 * Conformément à la documentation de Nginx, contrairement aux en-têtes, le *timeout* du corps de la requête doit
	 * s'appliquer entre chaque opération de lecture plutôt que sur l'ensemble de la requête. Actuellement, le calcul
	 * du *timeout* se base sur `this->request.getRequestTime()`, qui correspond à l'heure à laquelle la requête a été 
	 * acceptée. Il serait préférable de recalculer le *timeout* en utilisant le temps de la dernière lecture pour
	 * s'assurer que la durée est réinitialisée après chaque lecture.
	 */
	/**
	 * Si le client n'a pas envoyé les en-têtes de la requête dans le délai imparti,
	 * une réponse d'erreur 408 (Request Timeout) est envoyée et la connexion est fermée.
	 */
	time_t now = getTimestamp();
	time_t max = WBS_REQUEST_TIMEOUT;
	Router *handler = this->_current_router;

	/**
	 * Étant donné que le serveur a besoin d'avoir reçu l'en-tête `Host` pour savoir 
	 * à quel serveur virtuel la requête est destinée, tant que cet en-tête n'est pas reçu,
	 * on utilise la configuration du serveur par défaut pour déterminer le timeout.
	 */
	if (!handler)
		handler = this->_server->getDefault()->getDefaultHandler();

	if (!this->_headers_received)
	{
		max = handler->getTimeout().header_timeout;

		if (now > this->request.getRequestTime().header + max)
		{		
			Logger::debug("Client timeout");
			if (this->response)
			{
				this->response->status(408).sendDefault().end();
			}
			return (true);
		}
	}
	else
	{
		max = handler->getTimeout().body_timeout;

		if (now > this->request.getRequestTime().body + max)
		{
			Logger::debug("Client timeout");
			if (this->response)
			{
				/**
				 * Dans le cas où le client a demandé une mise à niveau vers WebSocket, on envoie une trame
				 * de fermeture à la place d'une réponse HTTP.
				 * 
				 * Normalement, le protocole WebSocket n'a pas de timeout, mais comme l'implémentation est une expérimentation,
				 * on laisse le timeout pour éviter de bloquer le serveur pour rien.
				 */
				if (this->response->isUpgraded())
				{
					this->response->sendFrame("timeout", 1000).end();
				}
				else
				{
					this->response->status(408).sendDefault().end();
				}
			}
			return (true);
		}
	}
	return (false);
}
