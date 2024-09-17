/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:35:12 by mgama             #+#    #+#             */
/*   Updated: 2024/09/17 18:22:38 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Client.hpp"
#include "proxy/ProxyWorker.hpp"

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
	Logger::debug("new client on fd: "+toString(client));
}

Client::~Client(void)
{
	if (this->upgraded_to_proxy)
	{
		/**
		 * Lorsque le Client est passé en mode proxy, ce dernier passe le relais au ProxyWorker,
		 * et doit être detruit mais sans fermer la connexion avec le client.
		 * Dans ce cas on annule la réponse et on supprime le pointeur.
		 */
		this->response->cancel();
		Server::printResponse(this->request, *this->response, 0);
		delete this->response;
		Logger::debug("------------------Client upgraded to proxy and closed-------------------", B_YELLOW);
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
	Logger::debug("------------------Client closed-------------------", B_YELLOW);
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
			// std::cout << "Router::route()" << *this->_current_router << std::endl;
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
		}

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
				if (ProxyWorker(this->_client, this->_current_router->getProxyConfig(), this->request, this->_buffer)())
				{
					/**
					 * S une erreur s'est produite lors de la création du ProxyWorker,
					 * on envoie une réponse d'erreur 502 (Bad Gateway) informant le client que la connection
					 * avec le serveur distant n'a pas pu être établie.
					 */
					this->response->status(502).sendDefault().end();
					return (WBS_ERR);
				}
				this->upgraded_to_proxy = true;
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
				this->response->status(408).sendDefault().end();
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
				this->response->status(408).sendDefault().end();
			return (true);
		}
	}
	return (false);
}
