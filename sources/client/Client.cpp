/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:35:12 by mgama             #+#    #+#             */
/*   Updated: 2024/04/15 19:59:10 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Client.hpp"

Client::Client(Server *server, const int client, sockaddr_in clientAddr):
	_server(server),
	_client(client),
	_clientAddr(clientAddr),
	request(client, clientAddr),
	response(NULL)
{
}

Client::~Client(void)
{
	if (this->response)
		delete this->response;
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

	if (this->processLines()) {
		return (WBS_POLL_CLIENT_CLOSED);
	}

	if (this->request.processFinished()) {
		this->_server->handleRouting(&this->request, this->response);
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
			response->status(400).end();
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
	}

	if (!this->_buffer.empty() && this->request.headersReceived()) {
		/**
		 * Si le client envoie un corps de requête alors que la requête n'en attend pas,
		 * on envoie une réponse d'erreur.
		 */
		if (this->request.bodyReceived() && !this->request.hasContentLength()) {
			response->status(411).end();
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
	if ((time_t)getTimestamp() > this->request.getRequestTime() + WBS_REQUEST_TIMEOUT) {
		Logger::debug("Client timeout");
		this->response->status(408).end();
		return (true);
	}
	return (false);
}