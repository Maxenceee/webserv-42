/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:34 by mgama             #+#    #+#             */
/*   Updated: 2024/01/08 01:33:41 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "MIMEType.hpp"

std::map<int, std::string>	Response::_res_codes = Response::initCodes();

Response::Response(const Server &server, int socket, const Request &req): _server(server), _sent(false)
{
	this->_socket = socket;
	this->_version = req.getVersion();
	this->_method = req.getMethod();
	this->_status = req.getSatus();
	this->_path = req.getPath();
	this->initCodes();
	this->setHeader("Server", "42-webserv");
	/**
	 * On vérifie si la requête la renvoyé aucune erreur de parsing.
	 */
	if (this->_status != 200)
	{
		this->end();
	}
}

std::map<int, std::string>	Response::initCodes()
{
	std::map<int, std::string>	codes;
	
	codes[100] = "Continue";
	codes[101] = "Switching Protocols";
	codes[200] = "OK";
	codes[201] = "Created";
	codes[204] = "No Content";
	codes[301] = "Moved Permanently";
	codes[302] = "Found";
	codes[310] = "Too many Redirects";
	codes[400] = "Bad Request";
	codes[401] = "Unauthorized";
	codes[403] = "Forbidden";
	codes[404] = "Not Found";
	codes[405] = "Method Not Allowed";
	codes[413] = "Payload Too Large";
	codes[418] = "I’m a teapot";
	codes[500] = "Internal Server Error";
	codes[505] = "HTTP Version not supported";
	return (codes);
}

Response::~Response(void)
{
	/**
	 * Si la réponse n'a pas été envoyé au client avant qu'elle ne soit
	 * détruite on l'envoie;
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
	this->setHeader("Content-Length", std::to_string(this->_body.size()));
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
		/**
		 * Lors que l'envoie d'un fichier il est préférable d'envoyer son
		 * type MIME via l'en-tête 'Content-Type'.
		 */
		this->setHeader("Content-Type", MimeTypes::getMimeType(getExtension(filepath)));
		this->_body = buffer.str();
	}
	else
	{
		this->sendNotFound();
	}
	return (*this);
}

Response	&Response::sendNotFound(void)
{
	this->status(404);
	this->setHeader("Content-Type", "text/html");
	this->send("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\"><title>Error</title></head><body><pre>Cannot "+this->_method+" "+this->_path+"</pre></body></html>");
	return (*this);
}

/**
 * A voir si on la garde
 */
// Response	&Response::render(const std::string filename)
// {
// 	if (!this->_static_dir.count(filename))
// 	{
// 		this->status(500);
// 		this->setHeader("Content-Type", "text/html");
// 		this->_body = "<!DOCTYPE html>\n<html><title>Error</title><body>There was an error finding your file</body></html>";
// 	}
// 	this->sendFile(this->_static_dir[filename]);
// 	return (*this);
// }

Response	&Response::end()
{
	if (!this->_sent)
	{
		std::string	res = this->prepareResponse();
		/**
		 * La fonction send() sert à écrire le contenu d'un descripteurs de fichiers, ici
		 * le descripteurs du client. À la difference de write, la fonction send est
		 * spécifiquement conçue pour écrire dans un socket. Elle offre une meilleure
		 * gestion de la l'écriture dans un contexte de travaille en réseau.
		 */
		int ret = ::send(this->_socket, res.c_str(), res.size(), 0);
		this->_sent = true;
		printf(B_YELLOW"------------------Response sent-------------------%s\n\n", RESET);
	}
	else
	{
		std::cerr << "Response error: cannot set header it was sent" << std::endl;
	}
	return (*this);
}

const std::string	Response::prepareResponse(void)
{
	std::string	res;

	/**
	 * La norme RFC impose que chaque réponse HTTP suive un modèle strict.
	 * Ligne de statut (Version, Code-réponse, Texte-réponse)
	 * En-tête de réponse
	 * [Ligne vide]
	 * Corps de réponse
	 * (https://www.rfc-editor.org/rfc/rfc7230.html#section-3.1.2)
	 */
	res = "HTTP/" + this->_version + " " + std::to_string(this->_status) + " " + this->getSatusName() + "\n";
	for (t_mapss::iterator it = this->_headers.begin(); it != this->_headers.end(); it++) {
		res += it->first + ": " + it->second + "\n";
	}
	for (t_mapss::iterator it = this->_cookie.begin(); it != this->_cookie.end(); it++) {
		res += "Set-Cookie: " + it->second + "\n";
	}
	res += "\n";
	res += this->_body + "\n";
	return (res);
}

Response	&Response::setHeader(const std::string header, const std::string value)
{
	this->_headers[header] = value;
	return (*this);
}

Response	&Response::setCookie(const std::string name, const std::string value, const CookieOptions &options)
{
	std::string cookieStr = name + "=" + value;

	cookieStr += "; path=" + (!options.path.empty() ? options.path : "/");

	if (!options.domain.empty()) {
		cookieStr += "; domain=" + options.domain;
	}

	if (options.maxAge >= 0) {
		cookieStr += "; Max-Age=" + std::to_string(options.maxAge);
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
