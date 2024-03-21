/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:33 by mgama             #+#    #+#             */
/*   Updated: 2024/03/21 15:36:29 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

// Request::Request(const Server &server, const std::string &str, int socket, sockaddr_in clientAddr): _server(server), _raw(str), _socket(socket), _clientAddr(clientAddr), _status(200), _host(""), _body(""), _port(80)
Request::Request(const Server &server, int socket, sockaddr_in clientAddr): _server(server), _socket(socket), _clientAddr(clientAddr), _status(200), _raw(""), _host(""), _body(""), _port(80)
{
	this->request_time = getTimestamp();
	this->_ip = getIPAddress(this->_clientAddr.sin_addr.s_addr);
}

Request::~Request(void)
{
}

void	Request::pushData(char *data, size_t len)
{
	std::cout << "push: " << data << std::endl;
	this->_raw.append(data, len);
}

void	Request::processRequest(void)
{
	// this->parse();
	std::cout << "process: " << this->_raw << std::endl;
}

int	Request::parse(void)
{
	size_t	i = 0;

	/**
	 * La norme RFC impose que chaque requête HTTP suive un modèle strict.
	 * Ligne de commande (Commande, URL, Version de protocole)
	 * En-tête de requête
	 * [Ligne vide]
	 * Corps de requête
	 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-3)
	 */
	if ((this->_status = this->getRequestLine(this->_raw, i)) != 200)
	{
		return (REQ_ERROR);
	}
	this->getRequestHeadersAndBody(this->_raw, i);
	this->getRequestQuery();
	this->getRequestCookies();
	return (REQ_SUCCESS);
}

int	Request::getRequestLine(const std::string &str, size_t &i)
{
	size_t		j;
	std::string	req_line;

	/**
	 * Une requête HTTP commmence par une ligne de commande
	 * (Commande, URL, Version de protocole), on s'assure qu'elle est correctement
	 * formée.
	 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-3.1.1)
	 */
	i = str.find_first_of('\n');
	req_line = str.substr(0, i);
	i += 1;
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
	return (this->getRequestVersion(req_tokens[2]));
}

int	Request::getRequestVersion(const std::string &str)
{
	std::string	http("HTTP/");

	if (str.compare(0, 5, http) == 0)
	{
		this->_version.assign(str, 5, 3); // on extrait la version du protocole de la requête
		// le serveur n'accepte que le version 1.1 du protocole HTTP.
		// Cette version date du début des années 2000 et offre moins de
		// fonctionnalitées que les versions plus récentes (2023, HTTP/3) mais sont, de fait,
		// plus facilement implémentables.
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
	 * On vérifie que la méthode de la requête n'est pas éronée.
	 * (https://www.rfc-editor.org/rfc/rfc7231#section-4)
	 */
	if (!contains(this->_server.getMethods(), this->_method))
		return (405);
	return (200);
}

std::string			Request::nextLine(const std::string &str, size_t &i)
{
	std::string		ret;
	size_t			j;

	if (i == std::string::npos)
		return ("");
	j = str.find_first_of('\n', i);
	ret = str.substr(i, j - i);
	if (ret[ret.size() - 1] == '\r')
		pop(ret);
	i = (j == std::string::npos ? j : j + 1);
	return (ret);
}

int	Request::getRequestHeadersAndBody(const std::string &str, size_t &i)
{
	std::string	key;
	std::string	value;
	std::string	line;

	/**
	 * Selon la norme RFC les en-têtes doivent suivrent un modèle précis (Nom-Du-Champs: [espace?] valeur [espace?])
	 * Le nom de l'en-tête doit avoir une majuscule, s'il contient plusieurs mots, ils
	 * doivent aussi avoir une majuscule et être liés par un tiré '-'. Les en-têtes sont sensibles à
	 * la case et chaque type d'en-tête a son format spécifique pour ses différentes valeurs.
	 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-3.2)
	 */
	while ((line = nextLine(str, i)) != "\r" && line != "")
	{
		key = readKey(line);
		value = readValue(line);
		this->_headers[key] = value;
	}
	this->getRequestHostname(this->_headers["Host"]);
	if (i != std::string::npos)
		this->_body = str.substr(i, std::string::npos);
	else
		Logger::warning("RFC warning: missing empty line after headers");
	return (REQ_SUCCESS);
}

int	Request::getRequestQuery(void)
{
	size_t		i;
	std::string	query = "";

	/**
	 * Le chaine de requête fait partie intégrante des URIs et respectent
	 * un format bien particulier. Les URIs se décomposent en plusieurs
	 * partie distinctes.
	 * 
	 * - Le schéma d'URI (URI Scheme) est une lettre suivie de n'importe quelle combinaison de lettres,
	 * de chiffres, du signe plus (+), du point (.) ou d'un tiret (-) et se termine par deux points (:). Ici 'http:'.
	 * - On ajoute (//), chaîne de caractères pour les protocoles dont la requête comprend un chemin d'accès.
	 * - La partie hiérarchique est prévue pour contenir les informations d'identification de la ressource,
	 * hiérarchique par nature. En général suivi par le domaine puis un chemin optionnel.
	 * - La chaine de requête (Query) est une partie optionnelle séparée par un point d'interrogation (?) qui contient
	 * des informations complémentaires qui ne sont pas de nature hiérarchique, mais est souvent formée
	 * d'une suite de paires <clef>=<valeur> séparées par des points virgules ou par des esperluettes.
	 * (https://www.rfc-editor.org/rfc/rfc6920.html#section-3)
	 */
	i = this->_path.find_first_of('?');
	if (i != std::string::npos)
	{
		query.assign(this->_path, i + 1, std::string::npos);
		this->_path = this->_path.substr(0, i);
	}
	if (!query.size())
		return (REQ_SUCCESS);
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
	return (REQ_SUCCESS);
}

int	Request::getRequestHostname(const std::string &host)
{
	size_t	i;

	/**
	 * La norme RFC impose que pour chaque requête l'en-tête `Host` soit présent
	 * afin de fournir les informations sur l'hôte et le port à partir de l'URI cible,
	 * permettant au serveur d'origine de distinguer entre les ressources tout en
	 * traitant les demandes pour plusieurs noms d'hôte sur une seule adresse IP.
	 * 
	 * Sachant qu'une seule machine hôte peut héberger plusieurs serveurs, l'en-tête
	 * permet au serveur de savoir à quel domaine (nom d'hôte) et port vous souhaitez accéder.
	 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-5.4)
	 */
	if (!host.size())
	{
		this->_status = 400;
		return (REQ_ERROR);
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
	return (REQ_SUCCESS);
}

int	Request::getRequestCookies(void)
{
	std::vector<std::string>::iterator	it;

	std::string	cookies_string = this->_headers["Cookie"];
	if (!cookies_string.size())
		return (REQ_SUCCESS);

	std::vector<std::string> cookies = split(cookies_string, ';');
	
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
	return (REQ_SUCCESS);
}
