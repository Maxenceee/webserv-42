/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:53:09 by mgama             #+#    #+#             */
/*   Updated: 2024/11/11 13:41:24 by mgama            ###   ########.fr       */
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
	for (size_t i = 0; i < this->_server_name.size(); ++i) {
		if (this->_server_name[i].name == name && (this->_server_name[i].port == port || this->_server_name[i].port == -1)) {
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
