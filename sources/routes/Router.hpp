/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:04:59 by mgama             #+#    #+#             */
/*   Updated: 2024/01/06 17:45:47 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "Server.hpp"
#include "request/Request.hpp"

class Server;
class Request;

class Router
{
private:
	const Server				&_server;
	bool						_strict;
	bool						_aliasing;
	std::string					_path;
	std::string					_root;
	std::vector<std::string>	_allowed_methods;
	std::string					_active_dir;
	std::string					_index;

	void	removeTrailingSlash(std::string &str);

public:
	Router(const Server &server, const std::string path, const bool strict = false, const bool alias = false);
	~Router(void);

	void	use(const Request &req) const;

	void	allowMethod(const std::string method);
	void	allowMethod(const std::vector<std::string> method);

	// void	setActiveDir(const std::string dirpath);
	void	setRoot(const std::string path);
	void	setAlias(const std::string path);

	bool	isValidMethod(const std::string method) const;
};
