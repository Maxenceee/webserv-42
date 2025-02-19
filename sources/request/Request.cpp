/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 01:17:29 by mgama             #+#    #+#             */
/*   Updated: 2024/12/25 19:03:45 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request(int socket, sockaddr_in clientAddr, bool sslEnabled):
	_request_line_received(false),
	_headers_received(false),
	_body_received(false),
	_status(200),
	_socket(socket),
	_host(""),
	_port(80),
	_clientAddr(clientAddr),
	_transfert_encoding(false),
	_body(""),
	_body_size(0)
{
	/**
	 * Sauvegarde du temps lors de la réception de la requête, il sert au
	 * serveur à calculer le temps qu'il reste au client pour envoyer les en-têtes.
	 */
	this->request_time.header = getTimestamp();
	this->request_time.body = 0;
	this->_ip = getIPAddress(this->_clientAddr.sin_addr.s_addr);
	/**
	 * Ajout du port par defaut dans le cas où le client ne l'indique pas
	 * dans l'en-tête `Host`.
	 */
	this->_port = sslEnabled ? 443 : 80;
}

Request::~Request(void)
{
}

int	Request::processLine(const std::string &line)
{
	if (!this->_request_line_received)
	{
		/**
		 * La norme RFC impose que chaque requête HTTP suive un modèle strict :
		 * Ligne de commande (Commande, URL, Version de protocole)
		 * En-tête de requête
		 * [Ligne vide]
		 * Corps de requête
		 * 
		 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-3)
		 */
		if ((this->_status = this->getRequestLine(line)) == 200)
		{
			this->_request_line_received = true;
		}
	}
	else
	{
		if (!this->_headers_received)
		{
			if (line.empty())
			{
				this->_headers_received = true;
				this->request_time.body = getTimestamp();
				this->getRequestCookies();

				/**
				 * On s'assure que l'en-tête `Host` est présent dans la
				 * requête. Le cas échéant, la requête est invalide.
				 */
				if (this->_host.empty())
				{
					return (WBS_REQ_ERROR);
				}

				/**
				 * On vérifie si la requête contient un corps. Si c'est le cas, on
				 * en extrait la taille ou on vérifie si le codage `chunked` est utilisé.
				 */
				if (this->_method == "GET" || this->_method == "HEAD" || (!this->_headers.count("Content-Length") && !this->_headers.count("Transfer-Encoding")))
				{
					this->_body_received = true;
				}
				else if (this->_headers.count("Transfer-Encoding") && this->_headers["Transfer-Encoding"] == "chunked")
				{
					this->_transfert_encoding = true;
				}
				else if (this->_headers.count("Content-Length"))
				{
					/**
					 * On s'assure que la taille du corps de la requête est valide.
					 */
					if (!isNumber(this->_headers["Content-Length"]))
					{
						return (WBS_REQ_ERROR);
					}

					this->_body_size = std::atoi(this->_headers["Content-Length"].c_str());
					if (this->_body_size <= 0)
					{
						this->_body_received = true;
					}
				}
				return (WBS_REQ_SUCCESS);
			}

			/**
			 * Selon la norme RFC, les en-têtes doivent suivre un modèle précis (Nom-Du-Champ: [espace?] valeur [espace?]).
			 * Le nom de l'en-tête doit commencer par une majuscule. S'il contient plusieurs mots, ils
			 * doivent aussi commencer par une majuscule et être liés par un tiret (-). Les en-têtes sont sensibles à
			 * la casse et chaque type d'en-tête a son format spécifique pour ses différentes valeurs.
			 * 
			 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-3.2)
			 */
			std::string	key = readKey(line);
			if (key.empty())
			{
				Logger::error("request error: cannot parse header line");
				return (WBS_REQ_ERROR);
			}
			std::string	value = readValue(line);
			
			/**
			 * L'en-tête `Content-Length` indique la taille du corps de la requête en octets.
			 * On vérifie si elle est déjà présente dans les en-têtes, si c'est le cas, on s'assure
			 * que la valeur est la même que celle déjà présente.
			 */
			if (key == "Content-Length")
			{
				if (!isNumber(value))
					return (WBS_REQ_ERROR);

				if (this->_headers.count("Content-Length") && this->_headers["Content-Length"] != value)
					return (WBS_REQ_ERROR);
			}

			this->_headers[key] = value;

			/**
			 * Selon la norme RFC, les en-têtes `Transfer-Encoding` et `Content-Length` ne doivent pas
			 * être présents dans la même requête. Si c'est le cas, la requête est invalide.
			 */
			if (this->_headers.count("Transfer-Encoding") && this->_headers.count("Content-Length"))
			{
				return (WBS_REQ_ERROR);
			}

			/**
			 * On vérifie si l'en-tête `Host` est présent dans la requête. Si c'est le cas, on extrait
			 * le nom d'hôte et le port de l'en-tête.
			 */
			if (this->_headers.count("Host") && this->_host.empty())
			{
				this->getRequestHostname(this->_headers["Host"]);
			}
		}
		else if (!this->_body_received)
		{
			/**
			 * Sauvegarde du temps de la dernière lecture du corps de la requête.
			 * Le serveur doit se baser sur le temps de la dernière lecture pour
			 * calculer le temps d'attente (timeout) avant de fermer la connexion.
			 */
			this->request_time.body = getTimestamp();

			/**
			 * Si le codage `chunked` est utilisé, on traite le corps de la requête
			 * en conséquence.
			 */
			if (this->_transfert_encoding)
			{
				switch (this->processChunk(line))
				{
				case WBS_CHUNK_PROCESS_ERROR:
					return (WBS_REQ_ERROR);
				case WBS_CHUNK_PROCESS_ZERO:
					this->_body_received = true;
				case WBS_CHUNK_PROCESS_OK:
					break;
				}
				return (WBS_REQ_SUCCESS);
			}

			this->_body += line;
			if (this->_body.size() == this->_body_size)
			{
				this->_body_received = true;
			}
			else if (this->_body.size() > this->_body_size)
			{
				return (WBS_REQ_ERROR);
			}
		}
	}
	return (WBS_REQ_SUCCESS);
}

bool	Request::processFinished(void) const
{
	return (this->_headers_received && this->_body_received);
}

int	Request::getRequestLine(const std::string &req_line)
{
	size_t		j;

	/**
	 * Une requête HTTP commence par une ligne de commande
	 * (Commande, URL, Version de protocole), on s'assure qu'elle est correctement
	 * formée.
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-3.1.1)
	 */
	j = req_line.find_first_of(' ');
	if (j == std::string::npos)
	{
		Logger::error("RFC error: no space after method");
		return (400);
	}
	this->_method.assign(req_line, 0, j); // on extrait la méthode de la requête
	return (this->getRequestPath(req_line));
}

int	Request::getRequestPath(const std::string &str)
{
	std::vector<std::string>	req_tokens;

	req_tokens = split(str, ' ');
	if (req_tokens.size() < 3)
	{
		Logger::error("RFC error: missing PATH or HTTP version");
		return (400);
	}
	this->_path = decodeURIComponent(req_tokens[1]); // on extrait le chemin de la requête
	this->_raw_path = this->_path;
	this->getRequestQuery();
	return (this->getRequestVersion(req_tokens[2]));
}

int	Request::getRequestVersion(const std::string &str)
{
	std::string	http("HTTP/");

	if (str.compare(0, 5, http) == 0)
	{
		this->_version.assign(str, 5, 3); // on extrait la version du protocole de la requête
		/**
		 * Cette version date du début des années 2000 et offre moins de
		 * fonctionnalités que les versions plus récentes (HTTP/2 et HTTP/3) mais est, de fait,
		 * plus facilement implémentable.
		 */
		if (this->_version != "1.1")
		{
			Logger::error("request error: unsupported HTTP version");
			return (505);
		}
	}
	else
	{
		Logger::error("RFC error: invalid HTTP version");
		return (400);
	}
	/**
	 * On vérifie que la méthode de la requête n'est pas erronée.
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc7231#section-4)
	 */
	if (!contains(Server::methods, this->_method))
		return (405);
	return (200);
}

ChunkProcessResult	Request::processChunk(const std::string &chunks)
{
	/**
	 * Le codage `chunked` modifie le corps de la requête afin de le transférer sous forme d'une série de fragments,
	 * chacun avec son propre indicateur de taille, suivi d'une partie facultative de fin (trailer) contenant
	 * des champs d'en-tête d'entité. Cela permet de transférer du contenu produit dynamiquement avec les
	 * informations nécessaires pour que le destinataire puisse vérifier qu'il a reçu le message complet.
	 * 
	 * *fragment
	 * dernier-fragment
	 * trailer
	 * 
	 * fragment = taille-du-fragment (1*HEX)
	 *            données-du-fragment
	 * 
	 * dernier-fragment = 1*("0")
	 * 
	 * Le champ taille-du-fragment est une chaîne de chiffres hexadécimaux indiquant la taille du fragment. Le codage
	 * `chunked` se termine par n'importe quel fragment dont la taille est zéro, suivi du trailer, qui est terminé
	 * par une ligne vide.
	 * Le trailer permet à l'expéditeur d'inclure des champs d'en-tête HTTP supplémentaires à la fin du message. Le
	 * champ d'en-tête Trailer peut être utilisé pour indiquer quels champs d'en-tête sont inclus dans un trailer.
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc2616#section-3.6.1)
	 */

	size_t pos;
	size_t chunksize;

	this->_chunkBuffer.append(chunks);

	while (true) {
		pos = 0;
		chunksize = 0;

		// Trouver la fin de la taille du chunk
		size_t endPos = this->_chunkBuffer.find(WBS_CRLF, pos);
		if (endPos == std::string::npos) {
			// Fin du message atteinte
			break;
		}

		// Extraire la taille du chunk
		std::string chunkSizeStr = this->_chunkBuffer.substr(pos, endPos - pos);
		chunksize = strtol(chunkSizeStr.c_str(), NULL, 16);
		if (chunksize == 0 && chunkSizeStr == "0") {
			// Chunk de taille 0
			this->_chunkBuffer.clear();
			return (WBS_CHUNK_PROCESS_ZERO);
		} else if (chunksize == 0) {
			// Taille du chunk invalide
			return (WBS_CHUNK_PROCESS_ERROR);
		}

		// Trouver le début des données du chunk
		pos = endPos + 2;
		if (pos + chunksize + 2 > this->_chunkBuffer.size()) {
			// Le chunk est incomplet
			return (WBS_CHUNK_PROCESS_OK);
		}

		// Extraire les données du chunk
		std::string chunkData = this->_chunkBuffer.substr(pos, chunksize);

		// Vérifier si le chunk est complet
		if (chunkData.size() == chunksize) {
			// Le chunk est complet, ajoutez-le au corps de la requête
			this->_body.append(chunkData);
			// Réinitialiser le compteur de taille de données reçues
			this->_chunkBuffer.erase(0, pos + chunksize + 2);
		} else {
			// Le chunk est incomplet, attendez la suite des données
			return (WBS_CHUNK_PROCESS_OK);
		}
	}

	return (WBS_CHUNK_PROCESS_OK);
}

int	Request::getRequestQuery(void)
{
	size_t		i;
	std::string	query = "";

	/**
	 * La chaîne de requête fait partie intégrante des URIs et respecte
	 * un format bien particulier. Les URIs se décomposent en plusieurs
	 * parties distinctes.
	 * 
	 * - Le schéma d'URI (URI Scheme) est une lettre suivie de n'importe quelle combinaison de lettres,
	 * de chiffres, du signe plus (+), du point (.) ou d'un tiret (-) et se termine par deux points (:). Ici 'http:'.
	 * - On ajoute (//), chaîne de caractères pour les protocoles dont la requête comprend un chemin d'accès.
	 * - La partie hiérarchique est prévue pour contenir les informations d'identification de la ressource,
	 * hiérarchique par nature. En général suivie par le domaine puis un chemin optionnel.
	 * - La chaîne de requête (Query) est une partie optionnelle séparée par un point d'interrogation (?) qui contient
	 * des informations complémentaires qui ne sont pas de nature hiérarchique, mais est souvent formée
	 * d'une suite de paires <clef>=<valeur> séparées par des points-virgules ou par des esperluettes.
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc6920.html#section-3)
	 */
	i = this->_path.find_first_of('?');
	if (i != std::string::npos)
	{
		query.assign(this->_path, i + 1, std::string::npos);
		this->_path = this->_path.substr(0, i);
	}
	if (!query.size())
		return (WBS_REQ_SUCCESS);
	size_t pos = 0;
	while (pos < query.length()) {
		size_t ampersandPos = query.find('&', pos);

		std::string pair;
		if (ampersandPos != std::string::npos) {
			pair = query.substr(pos, ampersandPos - pos);
		} else {
			pair = query.substr(pos);
		}

		size_t equalPos = pair.find('=');

		std::string key = pair.substr(0, equalPos);
		std::string value = "";
		if (equalPos != std::string::npos) {
			value = pair.substr(equalPos + 1);
		}
		this->_query[key] = value;

		if (ampersandPos != std::string::npos) {
			pos = ampersandPos + 1;
		} else {
			break;
		}
	}
	return (WBS_REQ_SUCCESS);
}

int	Request::getRequestHostname(const std::string &host)
{
	size_t	i;

	/**
	 * La norme RFC impose que pour chaque requête, l'en-tête `Host` soit présent
	 * afin de fournir les informations sur l'hôte et le port à partir de l'URI cible,
	 * permettant au serveur d'origine de distinguer entre les ressources tout en
	 * traitant les demandes pour plusieurs noms d'hôte sur une seule adresse IP.
	 * 
	 * Sachant qu'une seule machine hôte peut héberger plusieurs serveurs, l'en-tête
	 * permet au serveur de savoir à quel domaine (nom d'hôte) et port vous souhaitez accéder.
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-5.4)
	 */
	if (!host.size())
	{
		return (WBS_REQ_ERROR);
	}
	i = host.find_first_of(':');
	this->_host = host.substr(0, i);
	if (i < std::string::npos)
	{
		this->_port = atoi(host.substr(i + 1).c_str());
	}
	else
	{
		this->_host = host;
	}
	return (WBS_REQ_SUCCESS);
}

int	Request::getRequestCookies(void)
{
	/**
	 * L'en-tête Cookie est utilisé pour envoyer des cookies du client vers le serveur. Il est composé
	 * de paires nom-valeur séparées par des points-virgules et des espaces.
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc6265.html#section-5.4)
	 */
	std::vector<std::string>::iterator	it;

	if (!this->_headers.count("Cookie"))
		return (WBS_REQ_SUCCESS);

	std::vector<std::string> cookies = split(this->_headers["Cookie"], ';');
	for (it = cookies.begin(); it != cookies.end(); it++)
	{
		std::string c = trim(*it, ' ');
		size_t equalPos = c.find('=');

		std::string key = c.substr(0, equalPos);
		std::string value = "";
		if (equalPos != std::string::npos) {
			value = c.substr(equalPos + 1);
		}
		this->_cookie[key] = value;
	}
	return (WBS_REQ_SUCCESS);
}

bool	Request::headersReceived(void) const
{
	return (this->_headers_received);
}

bool	Request::bodyReceived(void) const
{
	return (this->_body_received);
}

bool	Request::hasContentLength(void) const
{
	return (this->_headers.count("Content-Length"));
}

void	Request::setMethod(const std::string &method)
{
	this->_method = method;
}

void	Request::updateHost(const std::string &host)
{
	this->_headers["Host"] = host;
}

void	Request::setBody(const std::string &body)
{
	this->_body_received = true;
	this->_body_size = body.length();
	this->_body = body;
}

void	Request::setHeader(const std::string &header, const std::string &value)
{
	this->_headers[header] = value;
}

void	Request::removeHeader(const std::string &header)
{
	if (this->_headers.count(header))
	{
		this->_headers.erase(header);
	}
}

void	Request::clearHeaders(void)
{
	this->_headers.clear();
}
