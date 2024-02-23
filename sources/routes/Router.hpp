/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:04:59 by mgama             #+#    #+#             */
/*   Updated: 2024/02/23 20:27:03 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "webserv.hpp"
#include "server/Server.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"

class Server;
class Request;
class Response;

struct s_Router_Root {
	bool			set;
	bool			isAlias;
	std::string		path;
};

struct s_Router_Redirection {
	bool			enabled;
	int				status;
	std::string		path;
};

struct s_Router_Location {
	std::string		path;
	std::string		modifier;
	bool			strict;
};

class Router
{
private:
	Server							&_server;
	bool							_autoindex;
	struct s_Router_Location		_location;
	struct s_Router_Root			_root;
	struct s_Router_Redirection		_redirection;
	std::vector<std::string>		_allowed_methods;
	std::vector<std::string>		_index;
	std::map<int, std::string>		_error_page;

	regmatch_t						*_match;

	std::string	&checkLeadingTrailingSlash(std::string &str);
	
	const std::string		getDirList(const std::string dirpath, std::string reqPath);

	static std::map<std::string, void (Router::*)(Request &, Response &)>	_method_handlers;
	static std::map<std::string, void (Router::*)(Request &, Response &)>	initMethodHandlers();

	void	call(std::string method, Request &request, Response &response);
	void	handleGETMethod(Request &request, Response &response);
	void	handleHEADMethod(Request &request, Response &response);
	void	handlePOSTMethod(Request &request, Response &response);
	void	handlePUTMethod(Request &request, Response &response);
	void	handleDELETEMethod(Request &request, Response &response);

	bool	matchRoute(const std::string &route) const;
	
	std::string	getLocalFilePath(const std::string &requestPath);

public:
	Router(Server &server, const struct s_Router_Location location, const std::string parent_root = "/");
	~Router(void);

	void	route(Request &request, Response &response);

	void	allowMethod(const std::string method);
	void	allowMethod(const std::vector<std::string> method);

	// void	setActiveDir(const std::string dirpath);
	void	setRoot(const std::string path);
	void	setAlias(const std::string path);

	const std::string	getRoot(void) const;

	void	setRedirection(const std::string to, int status = 302);
	void	setAutoIndex(const bool autoindex);

	void	setIndex(const std::vector<std::string> index);
	void	addIndex(const std::string index);
	void	setErrorPage(const int code, const std::string path);

	bool	isValidMethod(const std::string method) const;

	void	print(std::ostream &os) const;
};

std::ostream	&operator<<(std::ostream &os, const Router &res);

#endif /* ROUTER_HPP */