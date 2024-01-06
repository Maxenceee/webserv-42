/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:05:17 by mgama             #+#    #+#             */
/*   Updated: 2024/01/06 17:59:23 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"
#include "response/Response.hpp"

Router::Router(const Server &server, const std::string path, const bool strict, const bool alias): _server(server), _path(path), _strict(strict), _aliasing(alias)
{
	removeTrailingSlash(this->_path);
	// std::cout << "router path: " << path << " normalized: " << this->_path << std::endl;
}

Router::~Router(void)
{
}

void	Router::allowMethod(const std::string method)
{
	if (this->isValidMethod(method))
		this->_allowed_methods.push_back(method);
	else
		std::cerr << B_RED"Invalid method found. No such `" << method << "`" << RESET << std::endl;
}

void	Router::allowMethod(const std::vector<std::string> method)
{
	for (std::vector<std::string>::const_iterator it = method.begin(); it != method.end(); it++)
		this->allowMethod(*it);
}

void	setActiveDir(const std::string dirpath)
{
	t_mapss	active_dir_files; // temp

	listFilesInDirectory(dirpath, active_dir_files);
	for (t_mapss::iterator it = active_dir_files.begin(); it != active_dir_files.end(); it++)
		std::cout << it->first << " -> " << it->second << std::endl;
}

void	Router::setRoot(const std::string path)
{
	if (this->_aliasing && this->_root.size()) {
		std::cout << "Aliasing is already enbaled for this router." << std::endl;
	} else if (this->_root.size()) {
		std::cout << B_RED"Root is already set, abording." << RESET << std::endl;
	} else {
		this->_root = path;
		this->_aliasing = false;
	}
	setActiveDir(path);
}

void	Router::setAlias(const std::string path)
{
	if (!this->_aliasing && this->_root.size()) {
		std::cout << B_RED"Overriding `root` directive." << RESET << std::endl;
	} else if (this->_root.size()) {
		std::cout << B_RED"Alias is already set, abording." << RESET << std::endl;
	} else {
		this->_root = path;
		this->_aliasing = true;
	}
	setActiveDir(path);
}

void	Router::use(const Request &req) const
{
	if (!this->isValidMethod(req.getMethod()) && this->_allowed_methods.size())
		return ;
	std::cout << "Root: " << this->_path << "\nUse path: " << req.getPath() << std::endl;

	Response response = Response(this->_server, req.getClientSocket(), req.getVersion(), req.getSatus());
	if (!response.canSend())
		return ;
	if (req.getPath().compare(0, this->_path.size(), this->_path) == 0)
	{
		std::cout << "valid route for " << req.getPath() << std::endl;
		std::string fullpath = this->_root + req.getPath().substr(this->_path.size());
		std::cout << "full path: " << fullpath << std::endl;
		response.sendFile(fullpath).end();
		std::cout << response << std::endl;
	}
}

bool	Router::isValidMethod(const std::string method) const
{
	return (std::find(this->_server.getMethods().begin(), this->_server.getMethods().end(), method) != this->_server.getMethods().end());
}

void	Router::removeTrailingSlash(std::string &str)
{
	if (str[0] != '/') {
		str.insert(0, "/");
	}
	if (str[str.size() - 1] == '/') {
		str.resize(str.size() - 1);
	}
}
