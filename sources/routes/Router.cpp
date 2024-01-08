/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:05:17 by mgama             #+#    #+#             */
/*   Updated: 2024/01/08 12:33:34 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"

Router::Router(const Server &server, const std::string path, const bool strict): _server(server), _path(path), _strict(strict)
{
	/**
	 * Par défault le router hérite du chemin de son parent. Celui peut être
	 * changé en appellant la méthode Router::setRoot ou Router::setAlias.
	 */
	this->_root.path = server.getRoot();
	this->_root.isAlias = false;
	this->_root.set = false;
	checkLeadingTrailingSlash(this->_path);
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
	if (this->_root.isAlias && this->_root.set) {
		std::cout << "Aliasing is already enbaled for this router." << std::endl;
	} else if (this->_root.set) {
		std::cout << B_RED"Root is already set, abording." << RESET << std::endl;
	} else if (!isDirectory(path)) {
		throw std::invalid_argument(B_RED"router error: Not a directory: "+path+RESET);
	} else {
		this->_root.path = path;
		this->_root.isAlias = false;
	}
	setActiveDir(path);
}

void	Router::setAlias(const std::string path)
{
	if (!this->_root.isAlias && this->_root.set) {
		std::cout << B_RED"Overriding `root` directive." << RESET << std::endl;
	} else if (this->_root.set) {
		std::cout << B_RED"Alias is already set, abording." << RESET << std::endl;
	} else if (!isDirectory(path)) {
		throw std::invalid_argument(B_RED"router error: Not a directory: "+path+RESET);
	} else {
		this->_root.path = path;
		this->_root.isAlias = true;
	}
	setActiveDir(path);
}

void	Router::setRedirection(const std::string to, bool permanent)
{
	/**
	 * Définit le chemin de redirection du router, le booléen `permanent`
	 * permet de spécifier le type de redirection (permanent ou temporaire),
	 * celui-ci affectant le code de réponse (respectivement 301 et 302).
	 */
	this->_redirection.path = to;
	this->_redirection.permanent = permanent;
	this->_redirection.enabled = true;
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
	if ((request.getPath().compare(0, this->_path.size(), this->_path) == 0 && !this->_strict) || (request.getPath() == this->_path))
	{
		/**
		 * Nginx n'est pas très clair quant au priorité entre `root`/`alias` et
		 * `return`. Nous avons fais le choix de toujours donner la priorité à
		 * return.
		 */
		if (this->_redirection.enabled) {
			std::cout << "redirect to: " << this->_redirection.path << std::endl;
			response.redirect(this->_redirection.path, this->_redirection.permanent);
			response.end();
			return ;
		}
		std::cout << "valid route for " << request.getPath() << std::endl;
		std::string fullpath = this->_root.path + request.getPath().substr(this->_path.size());
		std::cout << "full path: " << fullpath << std::endl;
		if (isDirectory(fullpath)) {
			if (isFile(fullpath+"/index.html"))
				response.sendFile(fullpath+"/index.html");
			else {
				std::cerr << "cannot get " << fullpath << std::endl;
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

void	Router::checkLeadingTrailingSlash(std::string &str)
{
	/**
	 * Nginx n'a pas de comportement spécifique dépendant de la présence ou
	 * non du '/' au début du chemin du router. Le chemins 'chemin' et '/chemin'
	 * ont le même comportement.
	 * Pour simplifier la suite nous l'ajoutons s'il est manquant.
	 */
	if (str[0] != '/') {
		str.insert(0, "/");
	}
	/**
	 * La présence du '/' à la fin influe sur le comportement si le router est
	 * configuré comme strict, les chemins '/chemin' et '/chemins/' n'ont pas le même
	 * comportement dans ce cas.
	 * Si le router n'est pas strict nous le supprimons s'il est présent pour
	 * simplifier la suite.
	 */
	if (str[str.size() - 1] == '/' && !this->_strict) {
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