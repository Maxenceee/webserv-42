/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:34:49 by mgama             #+#    #+#             */
/*   Updated: 2024/02/27 12:27:52 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.hpp"
#include "ServerConfig.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "routes/Router.hpp"
#include "logger/Logger.hpp"

class Router;
class Request;
class Response;
class ServerConfig;

class Server
{
private:
	int							_id;
	bool						_init;
	uint32_t					address;
	uint16_t					port;
	int							socket_fd;
	struct sockaddr_in			socket_addr;
	fd_set						_fd_set;

	std::vector<ServerConfig *>	_configs;

	// les comportements par default du serveur sont stocké dans un router spécifique
	// Router						*_default;
	// std::vector<Router*>		_routes;

	static std::vector<std::string>		methods;
	static std::vector<std::string>		initMethods();

	// void		handleRoutes(Request &req, Response &res);

public:
	Server(int id, uint16_t port = 80);
	~Server(void);

	const int	init(void);
	void		kill(void);

	const bool		isInit(void) const;
	const int		getSocketFD(void) const;

	void			*addConfig(ServerConfig *config);
	const uint16_t	getPort(void) const;
	
	const std::vector<std::string>	getMethods(void) const;
	
	void		handleRequest(const int client, sockaddr_in clientAddr);
	
	void		printResponse(const Request &req, const Response &res) const;
	void		print(std::ostream &os) const;

	static bool	isValidMethod(const std::string method);

	class ServerPortInUse : public std::exception
	{
	public:
		virtual const char* what() const throw();
	};
	class ServerInvalidPort : public std::exception
	{
	public:
		virtual const char* what() const throw();
	};
	class ServerNotInit : public std::exception
	{
	public:
		virtual const char* what() const throw();
	};
};

std::ostream	&operator<<(std::ostream &os, const Server &server);

#endif /* SERVER_HPP */