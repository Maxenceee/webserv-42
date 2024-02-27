/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:52:36 by mgama             #+#    #+#             */
/*   Updated: 2024/02/26 15:48:21 by mgama            ###   ########.fr       */
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
	Server						&_server;
	uint32_t					address;
	uint16_t					port;
	std::vector<std::string>	_server_name;

	// les comportements par default du serveur sont stocké dans un router spécifique
	Router						*_default;
	std::vector<Router*>		_routes;


public:
	ServerConfig(Server &server);
	~ServerConfig(void);

	void		handleRoutes(Request &req, Response &res);

	Router							&getDefaultHandler(void);
	void							use(Router *router);
	
	void							setName(const std::vector<std::string> name);
	void							addName(const std::string name);
	const std::vector<std::string>	getName(void) const;

	const bool			hasErrorPage(const int code) const;
	const std::string	getErrorPage(const int code) const;

	void	print(std::ostream &os) const;
};

#endif /* SERVERCONFIG_HPP */