/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:53:09 by mgama             #+#    #+#             */
/*   Updated: 2024/02/27 12:30:04 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

std::ostream	&operator<<(std::ostream &os, const ServerConfig &config)
{
	config.print(os);
	os << std::endl;
	return os;
}

ServerConfig::ServerConfig(Server *server): _server(server)
{
	this->_default = new Router(*this, (struct s_Router_Location){.path = "/"});
	this->port = 80;
	this->address = 0;
}

ServerConfig::~ServerConfig(void)
{
	for (std::vector<Router *>::iterator it = this->_routes.begin(); it != this->_routes.end(); it++)
		delete *it;
	delete this->_default;
}

void	ServerConfig::handleRoutes(Request &req, Response &res)
{
	for (std::vector<Router *>::iterator it = this->_routes.begin(); it != this->_routes.end(); it++) {
		(*it)->route(req, res);
		if (!res.canSend())
			break;
	}
	/**
	 * Dans le cas où la route demandée n'a pu être géré par aucun des routers du serveur,
	 * on renvoie la réponse par defaut. (Error 404, Not Found)
	 */
	if (res.canSend())
	{
		res.sendNotFound().end();
	}
}

Router	&ServerConfig::getDefaultHandler(void)
{
	return (*this->_default);
}

void	ServerConfig::use(Router *router)
{
	this->_routes.push_back(router);
}

void	ServerConfig::setAddress(const std::string address)
{
	if (!this->_server || !this->_server->isInit())
		this->address = setIPAddress(address);
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

const uint32_t	ServerConfig::getAddress(void) const
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

const uint16_t	ServerConfig::getPort(void) const
{
	return (this->port);
}

void	ServerConfig::setNames(const std::vector<std::string> name)
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
		int port = 80;
		if (pos != std::string::npos)
			port = std::atoi(name.substr(pos + 1).c_str());
		this->_server_name[port] = name.substr(0, pos);	
	}
}

bool	ServerConfig::evalName(const std::string name, const uint16_t port) const
{
	if (this->_server_name.count(port) > 0)
	{
		if (this->_server_name.at(port) == name)
			return (true);
	}
	return (false);
}

const bool			ServerConfig::hasErrorPage(const int code) const
{
	return (this->_default->getErrorPage().count(code) > 0);
}

const std::string	ServerConfig::getErrorPage(const int code) const
{
	return (this->_default->getErrorPage().at(code));
}

void	ServerConfig::print(std::ostream &os) const
{
	os << B_BLUE << "Config: " << RESET << "\n";
	os << B_CYAN << "Name: " << RESET;
	for (std::map<int, std::string>::const_iterator it = this->_server_name.begin(); it != this->_server_name.end(); it++)
	{
		os << it->second << ":" << it->first << " ";
	}
	os << "\n";
	os << B_ORANGE << "Default router: " << RESET << "\n";
	os << *this->_default;
	os << B_ORANGE << "Routers: " << RESET << "\n";
	for (std::vector<Router *>::const_iterator it = this->_routes.begin(); it != this->_routes.end(); it++) {
		os << **it;
	}
}
