/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:05:17 by mgama             #+#    #+#             */
/*   Updated: 2024/01/08 00:00:04 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"

Router::Router(const Server &server, const std::string path, const bool strict, const bool alias): _server(server), _path(path), _strict(strict), _aliasing(alias)
{
	/**
	 * Par default un router hérite du chemin de son parent. Celui peut être
	 * changé en appellant la méthode Router::setRoot ou Router::setAlias.
	 */
	this->_root = server.getRoot();
	removeTrailingSlash(this->_path);
}

Router::~Router(void)
{
}

void	Router::allowMethod(const std::string method)
{
	/**
	 * allowMethod() indique au router qu'elle méthode HTTP il doit servir. Si aucune méthode
	 * n'est spécifiée le router les accepte toutes.
	 */
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
	} else if (!isDirectory(path)) {
		throw std::invalid_argument(B_RED"router error: Not a directory: "+path+RESET);
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
	} else if (!isDirectory(path)) {
		throw std::invalid_argument(B_RED"router error: Not a directory: "+path+RESET);
	} else {
		this->_root = path;
		this->_aliasing = true;
	}
	setActiveDir(path);
}

void	Router::route(Request &request, Response &response)
{
	/**
	 * Permet de faire le routage.
	 * Dans un premier temps on s'assure que la méthode de la requête est autorisé
	 * sur le router.
	 */
	if (!this->isValidMethod(request.getMethod()) && this->_allowed_methods.size())
		return ;
	std::cout << "Root: " << this->_path << "\nUse path: " << request.getPath() << std::endl;

	/**
	 * Avant de faire quelque logique de ce soit on s'assure que la réponse n'a pas déjà été
	 * envoyé pour une quelconque raison.
	 */
	if (!response.canSend())
		return ;
	/**
	 * On compare le chemin du router et celui de la requête.
	 * Pour le moment seul le chemin absoluts sont gérés ainsi que le requête GET.
	 * TODO:
	 * Gérer correctement les chemins du router et les différentes méthodes.
	 */
	if (request.getPath().compare(0, this->_path.size(), this->_path) == 0)
	{
		std::cout << "valid route for " << request.getPath() << std::endl;
		std::string fullpath = this->_root + request.getPath().substr(this->_path.size());
		std::cout << "full path: " << fullpath << std::endl;
		if (isDirectory(fullpath)) {
			if (isFile(fullpath+"/index.html"))
				response.sendFile(fullpath+"/index.html");
			else {
				std::cerr << "cannot get " << fullpath << std::endl;
				// response.setHeader("Content-Type", "text/html");
				// response.send(this->getDirList(fullpath, request.getPath()));
				response.sendNotFound();
			}
		} else if (isFile(fullpath)) {
			response.sendFile(fullpath);
		} else {
			response.sendNotFound();
		}
		response.end();
	}
}

bool	Router::isValidMethod(const std::string method) const
{
	return (std::find(this->_server.getMethods().begin(), this->_server.getMethods().end(), method) != this->_server.getMethods().end());
}

void	Router::removeTrailingSlash(std::string &str)
{
	/**
	 * Normalise le chemin passé depuis la configuration en supprimant le '/' à la fin
	 * et en ajoutant un au début s'il est manquant.
	 */
	if (str[0] != '/') {
		str.insert(0, "/");
	}
	if (str[str.size() - 1] == '/') {
		str.resize(str.size() - 1);
	}
}

const std::string	Router::getDirList(const std::string dirpath, const std::string reqPath) const
{
	t_mapss		temp;
	std::string	res;

	listFilesInDirectory(dirpath, temp, false);
	res = "<!DOCTYPE html>\n<html><title>"+reqPath+"</title><body>";
	for (t_mapss::iterator it = temp.begin(); it != temp.end(); it++) {
		res += "<a href=\""+reqPath+"/"+it->first+"\">" + it->first + "</a>";
	}
	res += "</body></html>";
	return (res);
}