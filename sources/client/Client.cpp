/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:35:12 by mgama             #+#    #+#             */
/*   Updated: 2024/04/18 13:21:56 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Client.hpp"

Client::Client(Server *server, const int client, sockaddr_in clientAddr):
	_server(server),
	_client(client),
	_clientAddr(clientAddr),
	_headers_received(false),
	_current_router(NULL),
	request_time(getTimestamp()),
	request(client, clientAddr),
	response(NULL)
{
}

Client::~Client(void)
{
	if (this->response)
	{
		Server::printResponse(this->request, *this->response, getTimestamp() - this->request_time);
		delete this->response;
	}
	close(this->_client);
	Logger::debug(B_YELLOW"------------------Client closed-------------------\n");
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
		this->request_time = getTimestamp();
		return (WBS_POLL_CLIENT_CLOSED);
	}

	if (this->request.processFinished())
	{
		this->request_time = getTimestamp();
		/**
		 * Une fois que la requête est complètement parsée, on peut effectuer le routage.
		 */
		// std::cout << "Router::route()" << *this->_current_router << std::endl;
		this->_current_router->route(this->request, *this->response);
		/**
		 * Pour le moment, le server ne gère pas le keep-alive (en-tête connection).
		 * On indique donc au cluster qu'il faut fermer la connexion.
		 */
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

		if (!this->_headers_received && this->request.headersReceived())
		{
			this->_headers_received = true;
			/**
			 * Extraction du router correspondant à la requête.
			 */
			this->_current_router = this->_server->eval(this->request, *this->response);
			/**
			 * La fonction Server::eval() retourne un pointeur vers le router correspondant à la requête
			 * ou le router par défaut du server si aucun router correspondant n'a été trouvé, cette fonction
			 * n'est donc jamais censée retourner NULL, mais on sécurise tout de même.
			 */
			if (this->_current_router == NULL)
			{
				this->response->status(404).sendDefault().end();
				return (WBS_ERR);
			}

			/**
			 * Si lors de l'évaluation de la requête, on il y a eu une erreur, le code de statut de la réponse
			 * est alors différent de 200, on envoie donc une réponse d'erreur.
			 */
			if (response->getStatus() != 200)
			{
				this->_current_router->sendResponse(*this->response);
				return (WBS_ERR);
			}

			/**
			 * TODO:
			 * 
			 * Gerer le proxybuffering off;
			 * consiste à envoyer tout le contenu recu directement au server distant sans le stocker
			 */
			// if (this->_current_router->isProxy())
			// {
			// 	/**
			// 	 * TODO:
			// 	 * 
			// 	 * Setup le proxyworker
			// 	 */
			// }
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
			response->status(411).sendDefault().end();
			return (WBS_ERR);
		}

		if (request.processLine(this->_buffer))
		{
			// Dans le cas d'une erreur on envoie une réponse d'erreur
			response->status(400).end();
			return (WBS_ERR);
		}
		this->_buffer.clear();
		return (WBS_NOERR);
	}
	return (WBS_NOERR);
}

bool	Client::timeout(void) {
	if ((time_t)getTimestamp() > this->request.getRequestTime() + WBS_REQUEST_TIMEOUT)
	{
		Logger::debug("Client timeout");
		this->response->status(408).sendDefault().end();
		return (true);
	}
	return (false);
}
