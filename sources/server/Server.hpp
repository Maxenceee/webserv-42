/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:34:49 by mgama             #+#    #+#             */
/*   Updated: 2024/04/18 13:12:11 by mgama            ###   ########.fr       */
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
	uint16_t					port;
	uint32_t					_address;
	int							socket_fd;
	struct sockaddr_in			socket_addr;

	ServerConfig				*_default;
	std::vector<ServerConfig *>	_configs;

	// les comportements par default du serveur sont stocké dans un router spécifique
	// Router						*_default;
	// std::vector<Router*>		_routes;

	static std::vector<std::string>		initMethods();

	// void		handleRoutes(Request &req, Response &res);

public:
	Server(int id, uint16_t port = 80, uint32_t address = 0);
	~Server(void);

	int				init(void);
	void			kill(void);

	bool			isInit(void) const;
	int				getSocketFD(void) const;

	void			addConfig(ServerConfig *config);
	ServerConfig	*getDefault(void) const;

	uint16_t		getPort(void) const;
	uint32_t		getAddress(void) const;
	
	static std::vector<std::string>		methods;
	
	Router		*eval(Request &request, Response &response) const;
	// void		handleRouting(Request *request, Response *response);
	
	static void		printResponse(const Request &req, const Response &res, const double response_duration);
	void			print(std::ostream &os) const;

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