/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:04:59 by mgama             #+#    #+#             */
/*   Updated: 2024/01/11 19:55:41 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

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
	bool			permanent;
	std::string		path;
};

class Router
{
private:
	const Server					&_server;
	bool							_strict;
	bool							_autoindex;
	std::string						_path;
	struct s_Router_Root			_root;
	struct s_Router_Redirection		_redirection;
	std::vector<std::string>		_allowed_methods;
	std::string						_active_dir;
	std::string						_index;

	void	checkLeadingTrailingSlash(std::string &str);
	
	const std::string		getDirList(const std::string dirpath, std::string reqPath);

public:
	Router(const Server &server, const std::string path, const bool strict = false);
	~Router(void);

	void	route(Request &request, Response &response);

	void	allowMethod(const std::string method);
	void	allowMethod(const std::vector<std::string> method);

	// void	setActiveDir(const std::string dirpath);
	void	setRoot(const std::string path);
	void	setAlias(const std::string path);

	void	setRedirection(const std::string to, bool permanent = false);
	void	setAutoIndex(const bool autoindex);

	bool	isValidMethod(const std::string method) const;
};
