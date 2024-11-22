/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:35:12 by mgama             #+#    #+#             */
/*   Updated: 2024/11/22 11:47:34 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Client.hpp"
#include "proxy/ProxyWorker.hpp"
#include "base64.hpp"
#include "websocket/websocket.hpp"

Client::Client(Server *server, const int client, sockaddr_in clientAddr):
	_server(server),
	_client(client),
	_headers_received(false),
	_current_router(NULL),
	upgraded_to_proxy(false),
	request_time(getTimestamp()),
	request(client, clientAddr),
	response(NULL)
{
	Logger::debug("New client on fd: "+toString(client));
}

Client::~Client(void)
{
	if (this->response && this->upgraded_to_proxy)
	{
		/**
		 * Lorsque le Client est passé en mode proxy, ce dernier passe le relais au ProxyWorker,
		 * et doit être detruit mais sans fermer la connexion avec le client.
		 * Dans ce cas on annule la réponse et on supprime le pointeur.
		 */
		this->response->cancel();
		Server::printResponse(this->request, *this->response, getTimestamp() - this->request_time);
		delete this->response;
		/**
		 * On ferme la connexion avec le client car désormais c'est le ProxyWorker qui s'occupe de la communication.
		 */
		Logger::debug("------------------Client upgraded to proxy-------------------", B_YELLOW);
		return ;
	}
	if (this->response && this->response->isUpgraded())
	{
		/**
		 * Dans le cas ou le client a demandé une mise à niveau vers WebSocket, on ne ferme pas la connexion et
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
		if (Logger::_debug)
			std::cout << *this->response << std::endl;
		Server::printResponse(this->request, *this->response, getTimestamp() - this->request_time);
		delete this->response;
	}
	close(this->_client);
	Logger::debug("------------------Client connection closed-------------------", B_YELLOW);
}

int	Client::process(void)
{
	char buffer[WBS_RECV_SIZE] = {0};

	/**
	 * La fonction recv() sert à lire le contenu d'un descripteur de fichiers, ici
	 * le descripteur du client. À la difference de read(), la fonction recv() est
	 * spécifiquement conçue pour la lecture à partir de socket. Elle offre une meilleure
	 * gestion de la lecture dans un contexte de travail en réseau.
	 */
	int valread = recv(this->_client, buffer, sizeof(buffer), 0);
	if (valread == -1) {
		Logger::error("server error: an error occurred while reading from client");
		return (WBS_POLL_CLIENT_ERROR);
	} else if (valread == 0) {
		Logger::debug("Connection closed by the client");
		return (WBS_POLL_CLIENT_DISCONNECT);
	}

	this->_buffer.append(buffer, valread);

	if (this->processLines())
	{
		// La sauvegarde du temps est necessaire pour le calcul de la durée de la requête
		this->request_time = getTimestamp();
		return (WBS_POLL_CLIENT_CLOSED);
	}

	if (this->response && this->response->isUpgraded())
	{
		/**
		 * Dans le cas ou le client a demandé une mise à niveau vers WebSocket, on ne ferme pas la connexion et
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
			// this->_proxy = new ProxyClient(this->_client, this->_current_router->getProxyConfig());
			// if (this->_proxy->connect())
			// {
			// 	this->response->status(502).sendDefault().end();
			// 	delete this->_proxy;
			// 	this->_proxy = NULL;
			// 	return (WBS_POLL_CLIENT_ERROR);
			// }

			// /**
			//  * On ajoute le client du proxy au tableau des descripteurs à surveiller.
			//  */
			// this->_poll_fds.push_back((pollfd){this->_proxy->getSocketFD(), POLLIN, 0});

			// this->_poll_clients[this->_proxy->getSocketFD()] = (wbs_pollclient){WBS_POLL_PROXY, this->_proxy};

			// /**
			//  * On indique que le client est un proxy.
			//  */
			// this->is_proxy = true;

			// /**
			//  * On envoie la requête au serveur distant.
			//  */
			// if (this->_proxy->send(this->request))
			// {
			// 	this->response->status(502).sendDefault().end();
			// 	return (WBS_POLL_CLIENT_ERROR);
			// }
			// printf("request finished: new ProxyWorker !!!\n");
			return (WBS_POLL_CLIENT_CLOSED);
		}
		else
		{
			// La sauvegarde du temps est necessaire pour le calcul de la durée de la requête
			this->request_time = getTimestamp();
			/**
			 * Une fois que la requête est complètement parsée, on peut effectuer le routage.
			 */
			if (Logger::_debug)
				std::cout << this->request << std::endl;
			this->_current_router->route(this->request, *this->response);
			/**
			 * Pour le moment, le server ne gère pas le keep-alive (en-tête connection).
			 * On indique donc au cluster qu'il faut fermer la connexion.
			 */
		}
		return (WBS_POLL_CLIENT_CLOSED);
	}
	return (WBS_POLL_CLIENT_OK);
}

int	Client::processLines(void) {
	/**
	 * Dans le cas ou le client a demandé une mise à niveau vers WebSocket, evite de traiter la
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

		if (request.processLine(line))
		{
			// Dans le cas d'une erreur de parsing on envoie une réponse d'erreur
			this->response->status(400).end();
			return (WBS_ERR);
		}

		if (this->response == NULL)
		{
			this->response = new Response(this->_client, request);
			if (!this->response->canSend())
			{
				return (WBS_ERR);
			}
		}

		/**
		 * Extraction du router correspondant à la requête dès l'arrivée de l'en-tête `Host`.
		 */
		if (this->_current_router == NULL && this->request.hasHeader("Host"))
		{
			/**
			 * La fonction Server::eval() retourne un pointeur vers le router correspondant à la requête
			 * ou le router par défaut du server si aucun router correspondant n'a été trouvé, cette fonction
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
		 * On vérifie si les en-têtes ont été reçus. Au quel cas on peut traiter les cas de requêtes
		 * ne necessitant pas de corps.
		 */
		if (!this->_headers_received && this->request.headersReceived())
		{
			this->_headers_received = true;

			/**
			 * Si lors de l'évaluation de la requête, on il y a eu une erreur, le code de statut de la réponse
			 * est alors différent de 200, on envoie donc une réponse d'erreur.
			 */
			if (this->response->getStatus() != 200)
			{
				this->_current_router->sendResponse(*this->response);
				return (WBS_ERR);
			}
			
			if (this->request.hasHeader("Connection") && this->request.getHeader("Connection") == "Upgrade")
			{
				// /**
				//  * INFO:
				//  * Le serveur ne supporte pas le protocole WebSocket, on envoie donc une réponse d'erreur 501.
				//  */
				// this->response->status(501).sendDefault().end();
				// return (WBS_ERR);

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
				this->response->setHeader("Upgrade", "websocket");
				this->response->setHeader("Connection", "upgrade");
				this->response->setHeader("Sec-WebSocket-Accept", calculateSecWebSocketAccept(key));

				/**
				 * INFO:
				 * Pour supporter le protocole WebSocket, il faudrait créer un état supplémentaire, qui empecherait la
				 * fermeture de la connexion avec le client une fois le requête traitée.
				 * Dans le cas d'un serveur statique, une telle implementation n'a aucun interet puisque le serveur ne
				 * peut pas traiter les requêtes websocket.
				 */
				this->response->status(101).upgrade().end();
				return (WBS_NOERR);
			}

			/**
			 * TODO:
			 * 
			 * Gerer le proxybuffering on;
			 * consiste à stocker le contenu recu depuis le client jusqu'a ce que la taille du buffer soit atteinte 
			 * puis d'envoyer tout le buffer au serveur distant
			 * edit: pas sur de la faire
			 */
			if (this->_current_router->isProxy())
			{
				/**
				 * Si le router est un proxy, on crée un ProxyWorker qui va se charger de la communication
				 * avec le serveur distant.
				 */
				switch (ProxyWorker(this->_client, this->_current_router->getProxyConfig(), this->request, this->_buffer)())
				{
				/**
				 * Si une erreur s'est produite lors de la création du ProxyWorker,
				 * on envoie une réponse d'erreur 503 (Service Unavailable) informant le client que la connection
				 * avec le serveur distant n'a pas pu être établie.
				 */
				case WBS_PROXY_UNAVAILABLE:
					this->response->status(503).sendDefault().end();
					return (WBS_ERR);
				/**
				 * Ou le code 502 (Bad Gateway) informant qu'il y a eu une erreur lors de la communication
				 * avec le serveur distant.
				 */
				case WBS_PROXY_ERROR:
					this->response->status(502).sendDefault().end();
					return (WBS_ERR);
				case WBS_PROXY_OK:
					this->upgraded_to_proxy = true;
					break;
				}
				return (WBS_POLL_CLIENT_CLOSED);
			}
		}
	}

	if (!this->_buffer.empty() && this->request.headersReceived())
	{
		/**
		 * Si le client envoie un corps de requête alors que la requête n'en attend pas,
		 * on envoie une réponse d'erreur.
		 */
		if (this->request.bodyReceived() && !this->request.hasContentLength())
		{
			this->response->status(411).sendDefault().end();
			return (WBS_ERR);
		}

		if (request.processLine(this->_buffer))
		{
			// Dans le cas d'une erreur de parsing on envoie une réponse d'erreur
			this->response->status(400).end();
			return (WBS_ERR);
		}
		this->_buffer.clear();
		return (WBS_NOERR);
	}
	return (WBS_NOERR);
}

bool	Client::timeout(void) {
	/**
	 * TODO:
	 * Conformément à la documentation de Nginx, contrairement aux en-têtes, le *timeout* du corps de la requête doit
	 * s'appliquer entre chaque opération de lecture plutôt que sur l'ensemble de la requête. Actuellement, le calcul
	 * du *timeout* se base sur `this->request.getRequestTime()`, qui correspond à l'heure à laquelle la requête a été 
	 * acceptée. Il serait préférable de recalculer le *timeout* en utilisant le temps de la dernière lecture pour
	 * s'assurer que la durée est remise à zéro après chaque lecture.
	 */
	/**
	 * Si le client n'a pas envoyé les en-têtes de la requête dans le délai imparti,
	 * on envoie une réponse d'erreur 408 (Request Timeout) et on ferme la connexion.
	 */
	time_t now = getTimestamp();
	time_t max = WBS_REQUEST_TIMEOUT;
	Router *handler = this->_current_router;

	/**
	 * Étant donné que le server a besoin d'avoir reçu l'en-tête `Host` pour savoir 
	 * à quel server virtuel la requête est destinée, temps que cet en-tête n'est pas reçu,
	 * on utilise la configuration du server par défault pour déterminer le timeout.
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
				 * Dans le cas ou le client a demandé une mise à niveau vers WebSocket, on envoie une trame
				 * de fermeture à la place d'une réponse HTTP.
				 * 
				 * Normalement le protocol WebSocket n'a pas de timeout, mais comme l'implementation est une experimention
				 * on laisse le timeout pour eviter de bloquer le serveur pour rien.
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
