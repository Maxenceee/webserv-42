/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:34:49 by mgama             #+#    #+#             */
/*   Updated: 2024/02/25 17:03:22 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "routes/Router.hpp"
#include "logger/Logger.hpp"

class Router;
class Request;
class Response;

class Server
{
private:
	int							_id;
	bool						_init;
	uint32_t					address;
	uint16_t					port;
	std::string					_server_name;
	int							socket_fd;
	struct sockaddr_in			socket_addr;
	fd_set						_fd_set;

	// les comportements par default du serveur sont stocké dans un router spécifique
	Router						*_default;
	std::vector<Router*>		_routes;

	static std::vector<std::string>		methods;
	static std::vector<std::string>		initMethods();

	void		handleRoutes(Request &req, Response &res);

public:
	Server(int id, uint16_t port = 0);
	~Server(void);

	const int	init(void);
	void		setupRoutes(void);
	// const int	start(void);
	void		kill(void);

	const bool		isInit(void) const;
	const int		getSocketFD(void) const;

	void			setAddress(const std::string address);
	void			setAddress(const uint32_t address);
	const uint32_t	getAddress(void) const;

	void			setPort(const uint16_t port);
	const uint16_t	getPort(void) const;

	void				setName(const std::string name);
	const std::string	getName(void) const;
	
	const std::vector<std::string>	getMethods(void) const;
	Router							&getDefaultHandler(void);

	const bool			hasErrorPage(const int code) const;
	const std::string	getErrorPage(const int code) const;
	
	void		use(Router *router);

	void		handleRequest(const int client, sockaddr_in clientAddr);
	
	void		printResponse(const Request &req, const Response &res) const;
	void		print(std::ostream &os) const;

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