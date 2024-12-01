/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:34:49 by mgama             #+#    #+#             */
/*   Updated: 2024/12/01 18:24:34 by mgama            ###   ########.fr       */
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
	uint16_t					_port;
	uint32_t					_address;
	bool						_ssl_enabled;
	int							socket_fd;
	struct sockaddr_in			socket_addr;

	ServerConfig				*_default;
	std::vector<ServerConfig *>	_configs;

	static std::vector<std::string>		initMethods();

public:
	Server(int id, uint16_t port = 80, uint32_t address = 0, bool ssl = false);
	~Server(void);

	int				init(void);
	void			kill(void);

	bool			isInit(void) const;
	int				getSocketFD(void) const;

	void			addConfig(ServerConfig *config);
	ServerConfig	*getDefault(void) const;

	uint16_t		getPort(void) const;
	uint32_t		getAddress(void) const;

	bool			hasSSL(void) const;
	
	static std::vector<std::string>		methods;
	
	Router		*eval(Request &request, Response &response) const;

	ServerConfig	*getConfig(const char *name) const;
	ServerConfig	*getConfig(const std::string &name) const;
	
	static void		printResponse(const Request &req, const Response &res, const double response_duration);
	void			print(std::ostream &os) const;

	static bool	isValidMethod(const std::string &method);
};

std::ostream	&operator<<(std::ostream &os, const Server &server);

#endif /* SERVER_HPP */