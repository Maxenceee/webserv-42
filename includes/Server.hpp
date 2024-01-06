/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:34:49 by mgama             #+#    #+#             */
/*   Updated: 2024/01/06 17:02:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "readsocket/readsocket.hpp"
#include "request/Request.hpp"
#include "routes/Router.hpp"

class Router;
class Request;

class Server
{
private:
	uint16_t				port;
	int						socket_fd;
	struct sockaddr_in		socket_addr;
	fd_set					_fd_set;

	t_mapss					static_dir; // à potentiellement virer etant donné les Routers

	std::vector<Router>		_routes;

	static std::vector<std::string>		methods;
	static std::vector<std::string>		initMethods();

public:
	Server(int port);
	~Server(void);

	bool	exit;

	const int	init(void);
	void		setupRoutes(void);
	const int	start(void);
	void		kill(void);

	const uint16_t	getPort(void) const;

	void			setPort(const uint16_t port);

	int				setStaticDir(const std::string &path);
	const t_mapss	getStaticDir(void) const;
	
	const std::vector<std::string>	getMethods(void) const;

	void			handleRequest(const int client);
	void			handleRoutes(const Request &req);
};

void listFilesInDirectory(const std::string &path, t_mapss &fileMap);