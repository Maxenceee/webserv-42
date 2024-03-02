/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:52:36 by mgama             #+#    #+#             */
/*   Updated: 2024/03/02 18:41:34 by mgama            ###   ########.fr       */
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

struct s_Name {
	std::string		name;
	int				port;	
};

class ServerConfig
{
private:
	Server						*_server;
	uint32_t					address;
	uint16_t					port;
	std::vector<struct s_Name>	_server_name;

	// les comportements par default du serveur sont stocké dans un router spécifique
	Router						*_default;

public:
	ServerConfig(Server *server = NULL);
	~ServerConfig(void);

	void			setServer(Server *server);

	void			handleRoutes(Request &req, Response &res);

	Router			*getDefaultHandler(void);
	void			use(Router *router);

	void				setAddress(const std::string address);
	void				setAddress(const uint32_t address);
	const uint32_t		getAddress(void) const;

	void				setPort(const uint16_t port);
	const uint16_t		getPort(void) const;
	
	void								addNames(const std::vector<std::string> name);
	void								addName(const std::string name);
	const std::vector<struct s_Name>	&getNames(void) const;
	bool								evalName(const std::string name, const uint16_t port = 80) const;

	void	print(std::ostream &os) const;
};

std::ostream	&operator<<(std::ostream &os, const ServerConfig &config);

#endif /* SERVERCONFIG_HPP */