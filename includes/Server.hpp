/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:34:49 by mgama             #+#    #+#             */
/*   Updated: 2024/01/08 01:24:14 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "readsocket/readsocket.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "routes/Router.hpp"

class Router;
class Request;
class Response;

class Server
{
private:
	uint16_t				port;
	int						socket_fd;
	struct sockaddr_in		socket_addr;
	fd_set					_fd_set;
	std::string				_root;

	t_mapss					static_dir; // à potentiellement virer etant donné les Routers

	std::vector<Router>		_routes;

	static std::vector<std::string>		methods;
	static std::vector<std::string>		initMethods();

	void		handleRequest(const int client, sockaddr_in clientAddr);
	void		handleRoutes(Request &req, Response &res);

	void		use(const Router &router);

public:
	Server(int port);
	~Server(void);

	bool	exit;

	const int	init(void);
	void		setupRoutes(void);
	const int	start(void);
	void		kill(void);

	void			setPort(const uint16_t port);
	const uint16_t	getPort(void) const;

	int				setStaticDir(const std::string &path);
	const t_mapss	getStaticDir(void) const;
	
	const std::string				getRoot(void) const;
	const std::vector<std::string>	getMethods(void) const;
};

void listFilesInDirectory(const std::string &path, t_mapss &fileMap, bool recursive = true);