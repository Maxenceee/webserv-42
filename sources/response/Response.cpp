/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:34 by mgama             #+#    #+#             */
/*   Updated: 2024/06/19 11:54:05 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "MIMEType.hpp"

wbs_mapis_t	Response::http_codes = Response::initCodes();

Response::Response(int socket, const Request &req): _sent(false)
{
	this->_socket = socket;
	this->_version = req.getVersion();
	this->_method = req.getMethod();
	this->_status = req.getStatus();
	this->_path = req.getPath();
	this->setHeader("Server", WBS_SERVER_NAME);
	this->setHeader("X-Powered-By", "maxencegama");
	/**
	 * On vérifie si la requête n'a renvoyé aucune erreur de parsing.
	 */
	if (this->_status != 200)
	{
		this->end();
	}
}

wbs_mapis_t	Response::initCodes()
{
	wbs_mapis_t	codes;
	
	/**
	 * Information
	 */
	codes[100] = "Continue";
	codes[101] = "Switching Protocols";
	codes[102] = "Processing";
	/**
	 * Succès
	 */
	codes[200] = "OK";
	codes[201] = "Created";
	codes[202] = "Accepted";
	codes[203] = "Non-Authoritative Information";
	codes[204] = "No Content";
	codes[205] = "Reset Content";
	codes[206] = "Partial Content";
	/**
	 * Redirection
	 */
	codes[301] = "Moved Permanently";
	codes[302] = "Found";
	codes[303] = "See Other";
	codes[304] = "Not Modified";
	codes[305] = "Use Proxy";
	codes[307] = "Temporary Redirect";
	codes[308] = "Permanent Redirect";
	codes[310] = "Too many Redirects";
	/**
	 * Erreur du client
	 */
	codes[400] = "Bad Request";
	codes[401] = "Unauthorized";
	codes[402] = "Payment Required";
	codes[403] = "Forbidden";
	codes[404] = "Not Found";
	codes[405] = "Method Not Allowed";
	codes[406] = "Not Acceptable";
	codes[407] = "Proxy Authentication Required";
	codes[408] = "Request Time-out";
	codes[409] = "Conflict";
	codes[410] = "Gone";
	codes[411] = "Length Required";
	codes[412] = "Precondition Failed";
	codes[413] = "Payload Too Large";
	codes[414] = "Request-URI Too Long";
	codes[415] = "Unsupported Media Type";
	codes[416] = "Requested range unsatisfiable";
	codes[417] = "Expectation failed";
	codes[418] = "I'm a teapot"; // (https://www.rfc-editor.org/rfc/rfc2324#section-2.3.2) HTCPCP/1.0
	/**
	 * Erreur du serveur
	 */
	codes[500] = "Internal Server Error";
	codes[501] = "Not Implemented";
	codes[502] = "Bad Gateway";
	codes[503] = "Service Unavailable";
	codes[504] = "Gateway Time-out";
	codes[505] = "HTTP Version not supported";
	codes[506] = "Variant Also Negotiates";
	codes[507] = "Insufficient storage";
	codes[508] = "Loop detected";
	codes[509] = "Bandwidth Limit Exceeded";
	codes[510] = "Not extended";
	codes[511] = "Network authentication required";
	return (codes);
}

bool	Response::isValidStatus(int status)
{
	return (http_codes.count(status) != 0);
}

Response::~Response(void)
{
	/**
	 * Si la réponse n'a pas été envoyé au client avant qu'elle ne soit
	 * détruite on l'envoie.
	 */
	if (!this->_sent)
		this->end();
}

Response	&Response::status(const int status)
{
	this->_status = status;
	return (*this);
}

Response	&Response::send(const std::string data)
{
	this->_body = data;
	/**
	 * On ajoute l'en-tête `Content-Length` afin d'indiquer au client la taille de
	 * de la ressource.
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-3.3.2)
	 */
	this->setHeader("Content-Length", toString<int>(this->_body.size()));
	return (*this);
}

Response	&Response::sendFile(const std::string filepath)
{
	std::ofstream		file;
	std::stringstream	buffer;

	if (isFile(filepath))
	{
		file.open(filepath.c_str(), std::ifstream::in);
		if (file.is_open() == false)
		{
			this->sendNotFound();
			return (*this);
		}
		buffer << file.rdbuf();
		file.close();
		this->setHeader("Last-Modified", getLastModifiedDate(filepath));
		/**
		 * Lors de l'envoie d'un fichier il est préférable d'envoyer son
		 * type MIME via l'en-tête `Content-Type` afin de préciser au client
		 * à quel type de fichier/données il a à faire.
		 * 
		 * (https://www.rfc-editor.org/rfc/rfc2616#section-14.17)
		 */
		this->setHeader("Content-Type", MimeTypes::getMimeType(getExtension(filepath))); // +"; charset=utf-8"
		this->_body = buffer.str();
		/**
		 * On ajoute l'en-tête `Content-Length` afin d'indiquer au client la taille de
		 * de la ressource.
		 * 
		 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-3.3.2)
		 */
		this->setHeader("Content-Length", toString<int>(this->_body.size()));
	}
	else
	{
		this->sendNotFound();
	}
	return (*this);
}

Response	&Response::sendNotFound(const int code)
{
	this->status(code);
	this->setHeader("Content-Type", "text/html; charset=utf-8");
	std::string st("Cannot "+this->_method+" "+this->_path);
	this->send("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><title>"+st+"</title></head><body><pre>"+st+"</pre></body></html>");
	return (*this);
}

Response	&Response::sendDefault(const int code)
{
	if (code != -1)
		this->status(code);
	this->setHeader("Content-Type", "text/html; charset=utf-8");
	std::string st(toString<int>(this->_status)+" "+this->getSatusName());
	this->send("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><title>"+st+"</title></head><body><center><h1>"+st+"</h1></center><hr><center>"+WBS_SERVER_NAME+"</center></body></html>");
	return (*this);
}

Response		&Response::redirect(const std::string &path, int status)
{
	this->status(status);
	this->setHeader("Location", path);
	return (*this);
}

std::string		Response::getTime(void)
{
	// static const char	*wdays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	// static const char	*months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	// date += wdays[gmtm->tm_wday];
	// date += ", ";
    // date += (gmtm->tm_mday < 10 ? "0" : "") + toString(gmtm->tm_mday);
	// date += " ";
    // date += months[gmtm->tm_mon];
	// date += " ";
	// date += toString(1900 + gmtm->tm_year);
	// date += " ";
    // date += toString(gmtm->tm_hour) + ":" + toString(gmtm->tm_min) + ":" + toString(gmtm->tm_sec);
	// date += " GMT";
	/**
	 * Création de l'en-tête `Date` selon la norme. Une valeur de date HTTP
	 * représente l'heure en tant qu'instance de Universal Time Coordinated (UTC).
	 * La date indique UTC par l'abréviation de trois lettres pour
	 * Greenwich Mean Time, "GMT", un prédécesseur du standard UTC.
	 * Les valeurs au format asctime sont supposées être en UTC.
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc7231.html#section-7.1.1.1)
	 */

	time_t		now = time(0);
	tm			*gmtm = gmtime(&now); // récuperation du temps GMT
	std::string	date;
	char		buffer[80];

	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmtm);
	date = buffer;
	return (date);
}

Response	&Response::sendCGI(const std::string data)
{
	size_t	i = 0;
	size_t	j = data.size();

	if (data.empty()) {
		this->status(501).end();
		return (*this);
	}

	while (i < j) {
		size_t pos = data.find(WBS_CRLF, i);
		if (pos == std::string::npos) break;
		std::string line = data.substr(i, pos - i);
		
		if (line.find("Status: ") == 0) {
			this->status(std::atoi(line.substr(8, 3).c_str()));
		} else {
			size_t pos = line.find(": ");
			if (pos != std::string::npos) {
				this->setHeader(line.substr(0, pos), line.substr(pos + 2));
			}
		}
		i = pos + 2;
	}

	while (j > 0 && (data[j - 1] == '\r' || data[j - 1] == '\n')) {
		j -= 1;
	}

	this->send(data.substr(i, j - i));
	return (*this);
}

Response	&Response::end()
{
	if (!this->_sent)
	{
		this->setHeader("Date", this->getTime());
		/**
		 * On force l'ajout de l'en-tête `Content-Length` afin d'indiquer au client la taille de
		 * de la ressource.
		 */
		if (!this->_headers.count("Content-Length") && !this->_body.empty())
			this->setHeader("Content-Length", toString<int>(this->_body.size()));

		/**
		 * TODO:
		 * 
		 * Pour le moment, le server ne gère pas le keep-alive (en-tête connection).
		 * On envoie donc une réponse avec l'en-tête Connection: close pour fermer la connexion
		 * après chaque requête.
		 */
		this->setHeader("Connection", "close");

		/**
		 * La norme HTTP impose que certaines réponses ne doivent pas contenir de corps.
		 * 
		 * (https://www.rfc-editor.org/rfc/rfc7230#section-3.3.3)
		 */
		if (this->_status == 204 || this->_status == 304 || this->_status < 200)
			this->_body.clear();

		/**
		 * On formate la réponse HTTP.
		 */
		std::string	res = this->prepareResponse();

		/**
		 * La fonction send() sert à écrire le contenu d'un descripteur de fichiers, ici
		 * le descripteur du client. À la difference de write, la fonction send est
		 * spécifiquement conçue pour écrire dans un socket. Elle offre une meilleure
		 * gestion de la l'écriture dans un contexte de travail en réseau.
		 */
		int ret = ::send(this->_socket, res.c_str(), res.size(), 0);
		(void)ret;
		this->_sent = true;
		Logger::debug(B_YELLOW"------------------Response sent-------------------");
	}
	return (*this);
}

const std::string	Response::prepareResponse(void)
{
	std::string	res;

	/**
	 * Pour éviter les mauvaises surprises, on vérifie que le code de statut
	 * de la réponse est bien un code de statut HTTP valide.
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc7231.html#section-6)
	 */
	if (!this->http_codes.count(this->_status))
	{
		Logger::error("Response error: invalid status code");
		this->_status = 500;
	}

	/**
	 * Une réponse HTTP doit obligatoirement contenir une version HTTP valide.
	 * Dans le cas ou la requête n'a pas été parsée correctement, la version
	 * par défaut est la version 1.1.
	 */
	if (this->_version.empty())
	{
		Logger::error("Response error: invalid HTTP version");
		this->_version = "1.1";
	}

	/**
	 * La norme RFC impose que chaque réponse HTTP suive un modèle strict.
	 * Ligne de statut (Version, Code-réponse, Texte-réponse)
	 * En-tête de réponse
	 * [Ligne vide]
	 * Corps de réponse
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-3.1.2)
	 */
	res = "HTTP/" + this->_version + " " + toString(this->_status) + " " + this->getSatusName() + WBS_CRLF;
	for (wbs_mapss_t::iterator it = this->_headers.begin(); it != this->_headers.end(); it++) {
		res += it->first + ": " + it->second + WBS_CRLF;
	}
	for (wbs_mapss_t::iterator it = this->_cookie.begin(); it != this->_cookie.end(); it++) {
		res += "Set-Cookie: " + it->second + WBS_CRLF;
	}
	res += WBS_CRLF;
	if (this->hasBody())
		res += this->_body + WBS_CRLF;
	return (res);
}

Response	&Response::setHeader(const std::string header, const std::string value)
{
	if (!this->_sent)
		this->_headers[header] = value;
	else
		Logger::error("Response error: cannot set header after it was sent");
	return (*this);
}

bool	Response::canAddHeader(void) const
{
	/**
	 * Nginx offre la possibilité d'ajouter des en-têtes de réponses personnalisés
	 * en fonction du code de statut de la réponse.
	 * Cette fonction permet de vérifier si l'ajout d'en-tête est possible.
	 * 
	 * (https://nginx.org/en/docs/http/ngx_http_headers_module.html#add_header)
	 */
	return (
		this->_status == 200 ||
		this->_status == 201 ||
		this->_status == 204 ||
		this->_status == 206 ||
		this->_status == 301 ||
		this->_status == 302 ||
		this->_status == 303 ||
		this->_status == 304 ||
		this->_status == 307 ||
		this->_status == 308
	);
}

Response	&Response::setCookie(const std::string name, const std::string value, const wbs_cookie_options &options)
{
	/**
	 * L'en-tête `Set-Cookie` est envoyé par le serveur dans les réponses HTTP pour définir des cookies sur le client. Une
	 * fois reçu par le client, celui-ci stocke le cookie et l'envoie dans les futures requêtes HTTP vers le même domaine
	 * conformément aux directives spécifiées.
	 * Un serveur peut envoyer plusieurs en-têtes `Set-Cookie` dans une seule réponse HTTP pour définir plusieurs cookies
	 * ou plusieurs attributs pour un même cookie.
	 * Le cookie est constitué d'un nom et d'une valeur, séparés par le signe égal (=).
	 * 
	 * La RFC spécifie plusieurs attributs pouvant être associés à un cookie, tels que :
	 * - Expires (date d'expiration)
	 * - Max-Age (durée de validité en secondes)
	 * - Domain (domaine pour lequel le cookie est valide)
	 * - Path (chemin de l'URL pour lequel le cookie est valide)
	 * - Secure (indique si le cookie doit être envoyé uniquement via une connexion sécurisée HTTPS),
	 * - HttpOnly (indique si le cookie ne doit être accessible que via HTTP et non via des scripts JavaScript)
	 * - etc.
	 * 
	 * (https://www.rfc-editor.org/rfc/rfc6265.html#section-4.1)
	 */

	if (this->_sent)
	{
		Logger::error("Response error: cannot set header after it was sent");
		return (*this);
	}

	std::string cookieStr = name + "=" + value;

	cookieStr += "; path=" + (!options.path.empty() ? options.path : "/");
	if (!options.domain.empty()) {
		cookieStr += "; domain=" + options.domain;
	}

	if (options.maxAge >= 0) {
		cookieStr += "; Max-Age=" + toString(options.maxAge);
	}

	if (options.secure) {
		cookieStr += "; secure";
	}

	if (options.httpOnly) {
		cookieStr += "; HttpOnly";
	}

	this->_cookie[name] = cookieStr;
	return (*this);
}

Response		&Response::clearBody(void)
{
	this->_body.clear();
	return (*this);
}

bool		Response::hasBody(void) const
{
	return (this->_body.size() > 0);
}

void	Response::cancel(void)
{
	this->_sent = true;
}
