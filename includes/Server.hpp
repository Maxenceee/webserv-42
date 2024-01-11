/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:34:49 by mgama             #+#    #+#             */
/*   Updated: 2024/01/11 10:57:50 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "routes/Router.hpp"

class Router;
class Request;
class Response;

// struct s_Server_Error_Pages {
// 	bool				set;
// 	std::vector<int>	codes;
// 	std::string			path;
// };

class Server
{
private:
	uint16_t					port;
	int							socket_fd;
	struct sockaddr_in			socket_addr;
	fd_set						_fd_set;
	std::string					_root;
	std::map<int, std::string>	_error_page;

	// t_mapss						static_dir; // à potentiellement virer etant donné les Routers

	std::vector<Router>			_routes;

	static std::vector<std::string>		methods;
	static std::vector<std::string>		initMethods();

	void		handleRequest(const int client, sockaddr_in clientAddr);
	void		handleRoutes(Request &req, Response &res);

	void		use(const Router &router);

public:
	Server(int port);
	~Server(void);

	bool		exit;

	const int	init(void);
	void		setupRoutes(void);
	const int	start(void);
	void		kill(void);

	void			setPort(const uint16_t port);
	const uint16_t	getPort(void) const;

	// int				setStaticDir(const std::string &path);
	// const t_mapss	getStaticDir(void) const;
	
	const std::string				getRoot(void) const;
	const std::vector<std::string>	getMethods(void) const;

	void	setErrorPage(const int code, const std::string path);
};
