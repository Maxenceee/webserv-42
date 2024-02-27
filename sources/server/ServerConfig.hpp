/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:52:36 by mgama             #+#    #+#             */
/*   Updated: 2024/02/27 12:23:59 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "webserv.hpp"
#include "Server.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "routes/Router.hpp"
#include "logger/Logger.hpp"

class Router;
class Request;
class Response;

class ServerConfig
{
private:
	Server						*_server;
	uint32_t					address;
	uint16_t					port;
	std::map<int, std::string>	_server_name;

	// les comportements par default du serveur sont stocké dans un router spécifique
	Router						*_default;
	std::vector<Router*>		_routes;


public:
	ServerConfig(Server *server = NULL);
	~ServerConfig(void);

	void		handleRoutes(Request &req, Response &res);

	Router							&getDefaultHandler(void);
	void							use(Router *router);

	void				setAddress(const std::string address);
	void				setAddress(const uint32_t address);
	const uint32_t		getAddress(void) const;

	void				setPort(const uint16_t port);
	const uint16_t		getPort(void) const;
	
	void							setNames(const std::vector<std::string> name);
	void							addName(const std::string name);
	bool							evalName(const std::string name, const uint16_t port = 80) const;

	const bool			hasErrorPage(const int code) const;
	const std::string	getErrorPage(const int code) const;

	void	print(std::ostream &os) const;
};

std::ostream	&operator<<(std::ostream &os, const ServerConfig &config);

#endif /* SERVERCONFIG_HPP */