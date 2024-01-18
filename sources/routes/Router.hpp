/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:04:59 by mgama             #+#    #+#             */
/*   Updated: 2024/01/18 01:12:42 by mgama            ###   ########.fr       */
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

class Router
{
private:
	Server							&_server;
	bool							_strict;
	bool							_autoindex;
	std::string						_path;
	struct s_Router_Root			_root;
	struct s_Router_Redirection		_redirection;
	std::vector<std::string>		_allowed_methods;
	std::string						_index;
	std::map<int, std::string>		_error_page;

	void	checkLeadingTrailingSlash(std::string &str);
	
	const std::string		getDirList(const std::string dirpath, std::string reqPath);

public:
	Router(Server &server, const std::string path, const std::string parent_root = "/", const bool strict = false);
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

	void	setIndex(const std::string index);
	void	setErrorPage(const int code, const std::string path);

	bool	isValidMethod(const std::string method) const;

	void	print(std::ostream &os) const;
};

std::ostream	&operator<<(std::ostream &os, const Router &res);

#endif /* ROUTER_HPP */