/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:53:09 by mgama             #+#    #+#             */
/*   Updated: 2024/02/26 15:48:23 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

std::ostream	&operator<<(std::ostream &os, const ServerConfig &config)
{
	config.print(os);
	os << std::endl;
	return os;
}

ServerConfig::ServerConfig(Server &server): _server(server)
{
	this->_default = new Router(server, *this, (struct s_Router_Location){.path = "/"});
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

void	ServerConfig::setName(const std::vector<std::string> name)
{
	this->_server_name = name;
}

void	ServerConfig::addName(const std::string name)
{
	this->_server_name.push_back(name);
}

const std::vector<std::string>	ServerConfig::getName(void) const
{
	return (this->_server_name);
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
	os << B_CYAN << "Name: " << RESET << toStringl(this->_server_name) << "\n";
	os << B_ORANGE << "Default router: " << RESET << "\n";
	os << *this->_default;
	os << B_ORANGE << "Routers: " << RESET << "\n";
	for (std::vector<Router *>::const_iterator it = this->_routes.begin(); it != this->_routes.end(); it++) {
		os << **it;
	}
}
