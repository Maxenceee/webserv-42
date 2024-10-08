/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:53:09 by mgama             #+#    #+#             */
/*   Updated: 2024/06/20 16:44:09 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

std::ostream	&operator<<(std::ostream &os, const ServerConfig &config)
{
	config.print(os);
	os << std::endl;
	return os;
}

ServerConfig::ServerConfig(Server *server): _server(server), used(false)
{
	this->_default = new Router(NULL, wbs_router_location());
	/**
	 * On verifie si le serveur est lancé en tant que root ou non. Si c'est le cas, on
	 * configure le serveur pour que le port par defaut soit 80, sinon on utilise le port 8080.
	 */
	if (getuid() == 0)
		this->port = 80;
	else
		this->port = 8000;
	this->address = INADDR_ANY;
}

ServerConfig::~ServerConfig(void)
{
	delete this->_default;
	Logger::debug("ServerConfig destroyed");
}

void	ServerConfig::setServer(Server *server)
{
	this->_server = server;
}

// void	ServerConfig::handleRoutes(Request &req, Response &res)
// {
// 	std::vector<Router *>	&routes = this->_default->getRoutes();
// 	for (std::vector<Router *>::iterator it = routes.begin(); it != routes.end(); it++) {
// 		(*it)->route(req, res);
// 		if (!res.canSend())
// 			break;
// 	}
// 	/**
// 	 * Dans le cas où la route demandée n'a pu être géré par aucun des routers du serveur,
// 	 * on renvoie la réponse par defaut. (Error 404, Not Found)
// 	 */
// 	if (res.canSend())
// 	{
// 		/**
// 		 * Dans le cas ou le chemin de la requête ne correspond à aucune route, on vérifie si
// 		 * une redirection est définie par defaut pour le serveur. Si c'est le cas, on redirige
// 		 * la requête vers le chemin spécifié.
// 		 */
// 		if (this->_default->getRedirection().enabled) {
// 			if (this->_default->getRedirection().path.empty() && this->_default->getRedirection().data.empty()) {
// 				res.status(this->_default->getRedirection().status);
// 				res.sendDefault().end();
// 				return ;
// 			}
// 			if (this->_default->getRedirection().status % 300 < 100) {
// 				res.redirect(this->_default->getRedirection().path, this->_default->getRedirection().status).end();
// 			} else {
// 				res.status(this->_default->getRedirection().status).send(this->_default->getRedirection().data).end();
// 			}
// 			return ;
// 		}
// 		res.sendNotFound().end();
// 	}
// }

Router	*ServerConfig::getDefaultHandler(void)
{
	return (this->_default);
}

void	ServerConfig::use(Router *router)
{
	this->_default->use(router);
}

void	ServerConfig::setAddress(const std::string address)
{
	if (!this->_server || !this->_server->isInit())
	{
		if (address == "*")
			this->address = INADDR_ANY;
		else
			this->address = setIPAddress(address);
	}
	else
		Logger::error("server error: could not set address after server startup");
}

void	ServerConfig::setAddress(const uint32_t address)
{
	if (!this->_server || !this->_server->isInit())
		this->address = address;
	else
		Logger::error("server error: could not set address after server startup");
}

uint32_t	ServerConfig::getAddress(void) const
{
	return (this->address);
}

void	ServerConfig::setPort(const uint16_t port)
{
	if (!this->_server || !this->_server->isInit())
		this->port = port;
	else
		Logger::error("server error: could not set port after server startup");
}

uint16_t	ServerConfig::getPort(void) const
{
	return (this->port);
}

void	ServerConfig::addNames(const std::vector<std::string> name)
{
	for (std::vector<std::string>::const_iterator it = name.begin(); it != name.end(); it++)
	{
		this->addName(*it);
	}
}

void	ServerConfig::addName(const std::string name)
{
	size_t pos = name.find(':');
	if (pos != std::string::npos && pos != name.rfind(':'))
		Logger::error("server error: invalid server name: " + name);
	else {
		int port = -1;
		if (pos != std::string::npos)
			port = std::atoi(name.substr(pos + 1).c_str());
		this->_server_name.push_back((struct wbs_server_name){.name = name.substr(0, pos), .port = port});
	}
}

const wbs_server_names	&ServerConfig::getNames(void) const
{
	return (this->_server_name);
}

bool	ServerConfig::evalName(const std::string name, const uint16_t port) const
{
	/**
	 * Si le port n'est pas spécifié dans le nom du serveur, ceci sous-entend que le serveur
	 * accepte les requêtes sur tous les ports.
	 */
	for (size_t i = 0; i < _server_name.size(); ++i) {
		if (_server_name[i].name == name && (_server_name[i].port == port || _server_name[i].port == -1)) {
			return (true);
		}
	}
	return (false);
}

void	ServerConfig::print(std::ostream &os) const
{
	os << B_BLUE << "Config: " << RESET;
	if (this->_server->getDefault() == this)
		os << B_GREEN << "default" << RESET;
	os << "\n";
	os << B_CYAN << "Name: " << RESET;
	for (wbs_server_names::const_iterator it = this->_server_name.begin(); it != this->_server_name.end(); it++)
	{
		os << it->name;
		if (it->port > 0)
			os << ":" << it->port;
		os << " ";
	}
	os << "\n";
	os << B_ORANGE << "Default router: " << RESET << "\n";
	os << *this->_default;
}
