/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouterDirectives.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:05:17 by mgama             #+#    #+#             */
/*   Updated: 2024/12/24 11:56:10 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"

void	Router::setRoot(const std::string &path)
{
	/**
	 * Cette fonction indique au routeur son dossier racine, dossier à partir duquel il servira le contenu.
	 * Par défaut, il l'hérite de son parent.
	 * Pour simplifier, le chemin de la requête est ajouté à la fin du chemin de `root` pour former le
	 * chemin vers la ressource.
	 * (ex: routeur = /static, requête = /static/chemin/vers, root = ./public, chemin final => ./public/chemin/vers)
	 * 
	 * Attention, dans Nginx la directive `alias` a la priorité sur `root`, si `alias` est définie cette
	 * directive sera écrasée.
	 */
	if (this->_root.isAlias && this->_root.set) {
		Logger::info("router info: Aliasing is already enbaled for this router.");
	} else if (this->_root.set) {
		Logger::info("router info: Root is already set, abording.");
	} else {
		this->_root.set = true;
		this->_root.path = path;
		this->_root.nearest_root = this->_root.path;
		this->_root.isAlias = false;
		this->reloadChildren();
	}
}

void	Router::setAlias(const std::string &path)
{
	/**
	 * Permet de définir la directive `alias` pour ce routeur. Contrairement à `root`,
	 * la directive `alias` remplace le segment de l'URL correspondant par le chemin spécifié.
	 * (ex: routeur = /images, requête = /images/photo.jpg, alias = ./public/photos, chemin final => ./public/photos/photo.jpg)
	 * (dans ce cas root aurait donné: routeur = /images, requête = /images/photo.jpg, root = ./public/photos, chemin final => ./public/photos/images/photo.jpg)
	 * 
	 * Attention, dans Nginx la directive `alias` a la priorité sur `root`, si `root` a été définie
	 * précédemment cette dernière sera écrasée.
	 */
	if (!this->_root.isAlias && this->_root.set) {
		Logger::info("router info: Root is already set, abording.");
	} else {
		if (this->_root.set)
			Logger::info("router info: Overriding `root` directive.");
		this->_root.set = true;
		this->_root.path = path;
		this->_root.isAlias = true;
		this->reloadChildren();
	}
}

void	Router::setRedirection(const std::string &to, int status)
{
	/**
	 * Définit le chemin de redirection du routeur, le statut par défaut est 302 (Found).
	 * Si le chemin est vide alors le statut est retourné sans redirection.
	 */
	if (status % 300 < 100) {
		this->_redirection.path = to;
	} else {
		this->_redirection.data = to;
	}
	this->_redirection.status = status;
	this->_redirection.enabled = true;
}

void	Router::allowMethod(const std::string &method)
{
	/**
	 * Router::allowMethod() indique au routeur quelle méthode HTTP il doit servir. Si aucune méthode
	 * n'est spécifiée, le routeur les accepte toutes.
	 */
	if (Server::isValidMethod(method)) {
		if (!this->_allowed_methods.enabled) {
			this->_allowed_methods.enabled = true;
			this->_allowed_methods.methods.clear();
		}
		this->_allowed_methods.methods.push_back(method);
		this->reloadChildren();
	} else
		Logger::error("router error: Invalid method found. No such `" + method + "`");
}

void	Router::allowMethod(const std::vector<std::string> &method)
{
	for (std::vector<std::string>::const_iterator it = method.begin(); it != method.end(); it++)
		this->allowMethod(*it);
}

void	Router::setAutoIndex(const bool autoindex)
{
	this->_autoindex = autoindex;
}

void	Router::setIndex(const std::vector<std::string> &index)
{
	this->_index = index;
	this->reloadChildren();
}

void	Router::addIndex(const std::string &index)
{
	this->_index.push_back(index);
	this->reloadChildren();
}

void	Router::setHeader(const std::string &key, const std::string &value, const bool always)
{
	/**
	 * Cette méthode permet d'ajouter des en-têtes à la réponse du routeur.
	 * Si `always` est vrai, l'en-tête sera ajouté quel que soit le code de réponse.
	 * Les en-têtes sont héritées des niveaux de configuration précédents si et seulement si
	 * aucun n'est défini au niveau de configuration actuel. 
	 */
	if (!this->_headers.enabled) {
		this->_headers.enabled = true;
		this->_headers.list.clear();
	}
	this->_headers.list.push_back((wbs_router_header_t){key, value, always});
}

void	Router::setErrorPage(const int code, const std::string &path)
{
	if (code == 304) {
		Logger::warning("router warning: Cannot set error page for "+toString(code));
		return;
	}

	if (this->_error_page.count(code)) {
		Logger::info("router info: overriding previous error page for " + toString(code));
	}
	this->_error_page[code] = path;
	checkLeadingTrailingSlash(this->_error_page[code]);
}

void	Router::setClientMaxBodySize(const size_t size)
{
	if (this->_parent && this->_parent->hasClientMaxBodySize() && size >= this->_parent->getClientMaxBodySize()) {
		Logger::warning("router warning: Client max body size cannot be greater than parent's");
		return;
	}
	this->_client_body.size = size;
	this->_client_body.set = true;
	this->reloadChildren();
}

void	Router::setCGI(const std::string &path)
{
	this->_cgi.path = path;
	this->_cgi.enabled = true;
}

void	Router::enableCGI(void)
{
	this->_cgi.enabled = true;
}

void	Router::addCGIParam(const std::string &key, const std::string &value)
{
	this->_cgi.params[key] = value;
}

void	Router::setProxy(wbs_url &proxy_url)
{
	if (this->_proxy.enabled)
	{
		Logger::warning("router warning: Proxy is already enabled");
		return;
	}

	this->_proxy.enabled = true;
	this->_proxy.protocol = proxy_url.protocol;
	this->_proxy.host = proxy_url.host;
	this->_proxy.port = proxy_url.port;
	this->_proxy.path = proxy_url.path;
	this->reloadChildren();
}

void	Router::addProxyHeader(const std::string &key, const std::string &value)
{
	this->_proxy.headers[key] = value;
	this->reloadChildren();
}

void	Router::enableProxyHeader(const std::string &key)
{
	this->_proxy.forwared.push_back(key);
	this->reloadChildren();
}

void	Router::hideProxyHeader(const std::string &key)
{
	this->_proxy.hidden.push_back(key);
	this->reloadChildren();
}

void	Router::setProxyMethod(const std::string &method)
{
	this->_proxy.method = method;
	this->reloadChildren();
}

void	Router::setForwardRequestBody(bool enable)
{
	this->_proxy.forward_body = enable;
}

void	Router::setForwardRequestHeaders(bool enable)
{
	this->_proxy.forward_headers = enable;
}

void	Router::setTimeout(const size_t time, const std::string &type)
{
	if (type == "header") {
		this->_timeout.header_timeout = time;
		this->_timeout.header_set = true;
	}
	else if (type == "body") {
		this->_timeout.body_timeout = time;
		this->_timeout.body_set = true;
	}
	else
		throw std::invalid_argument("router error: Invalid usage of setTimeout()");
	this->reloadChildren();
}
