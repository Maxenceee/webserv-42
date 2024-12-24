/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:05:17 by mgama             #+#    #+#             */
/*   Updated: 2024/12/24 18:24:53 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"

std::ostream	&operator<<(std::ostream &os, const Router &router)
{
	router.print(os);
	os << std::endl;
	return os;
}

std::map<std::string, void (Router::*)(Request &, Response &)>	Router::_method_handlers = Router::initMethodHandlers();

std::map<std::string, void (Router::*)(Request &, Response &)>	Router::initMethodHandlers()
{
	std::map<std::string, void (Router::*)(Request &, Response &)>	map;

	map["GET"]		= &Router::handleGETMethod;
	map["HEAD"]		= &Router::handleHEADMethod;
	map["POST"]		= &Router::handlePOSTMethod;
	map["PUT"]		= &Router::handlePUTMethod;
	map["DELETE"]	= &Router::handleDELETEMethod;
	map["TRACE"]	= &Router::handleTRACEMethod;
	return (map);
}

Router::Router(Router *parent, const struct wbs_router_location &location, int level):
	_parent(parent),
	_location(location),
	_autoindex(false),
	level(level)
{
	/**
	 * Par défault le router hérite de la racine de son parent. Celui-ci peut être
	 * changé en appellant la méthode Router::setRoot() ou Router::setAlias().
	 */
	if (parent) {
		this->_root = parent->getRootData();
	} else {
		/**
		 * Le router par défaut n'héritant d'aucun autre router, doit être definit manuellement.
		 */
		this->_root.path = "/";
		this->_root.nearest_root = this->_root.path;
		this->_root.isAlias = false;
	}
	this->_root.set = false;

	/**
	 * Par défaut le router hérite des en-têtes de son parent. Celles-ci peuvent être
	 * changées en appellant la méthode Router::setHeader().
	 */
	this->_headers.enabled = false;
	if (parent) {
		this->_headers.list = parent->getHeaders();
	}

	/**
	 * Par défaut le router hérite des pages d'erreur de son parent.
	 */
	if (parent) {
		this->_error_page = parent->_error_page;
	}

	/**
	 * Par défaut le router hérite des pages d'index de son parent.
	 */
	if (parent) {
		this->_index = parent->_index;
	}

	/**
	 * Par défaut le router hérite des méthodes HTTP autorisées de son parent.
	 */
	this->_allowed_methods.enabled = false;
	if (parent) {
		this->_allowed_methods.methods = parent->_allowed_methods.methods;
	}
	
	this->_client_body.set = false;

	/**
	 * Par défaut le router hérite de la taille maximale du corps de la requête de son parent.
	 */
	if (parent && parent->hasClientMaxBodySize()) {
		this->_client_body.size = parent->getClientMaxBodySize();
		this->_client_body.set = true;
	} else {
		this->_client_body.size = 1024 * 1024; // 1MB
	}

	/**
	 * Par défaut le router hérite des timeouts de son parent.
	 */
	if (parent) {
		this->_timeout = parent->getTimeout();
	}

	this->_redirection.enabled = false;
	this->_cgi.enabled = false;

	/**
	 * Par défaut le router hérite de la configuration du proxy de son parent.
	 */
	if (parent) {
		this->_proxy = parent->getProxyConfig();
	}
}

Router::~Router(void)
{
	for (std::vector<Router *>::iterator it = this->_routes.begin(); it != this->_routes.end(); it++)
		delete *it;
	Logger::debug("Router destroyed");
}

void	Router::use(Router *router)
{
	/**
	 * Cette méthode permet d'ajouter un routeur enfant à ce routeur.
	 * Les routes du routeur enfant seront servies par ce routeur.
	 */
	this->_routes.push_back(router);
}

std::vector<Router *>	&Router::getRoutes(void)
{
	return (this->_routes);
}

Router	*Router::eval(const std::string &path, const std::string &method, Response &response)
{
	Router	*router = NULL;

	/**
	 * On vérifie si le chemin de la requête correspond à ce routeur.
	 * Si c'est le cas, on vérifie si la méthode HTTP est autorisée.
	 * Puis on évalue les routes enfants.
	 */
	if (this->matchRoute(path, response))
	{
		if (this->_allowed_methods.methods.size() && !contains(this->_allowed_methods.methods, method))
		{
			response.setHeader("Allow", Response::formatMethods(this->_allowed_methods.methods));
			response.status(405);
			return (this);
		}
		for (std::vector<Router *>::iterator it = this->_routes.begin(); it != this->_routes.end(); it++) {
			if ((router = (*it)->eval(path, method, response)) != NULL)
				return (router);
		}
		router = this;
	}
	return (router);
}

void	Router::sendResponse(Response &response)
{
	if (response.canSend()) {
		for (std::vector<wbs_router_header_t>::iterator it = this->_headers.list.begin(); it != this->_headers.list.end(); it++) {
			if (it->always || response.canAddHeader())
				response.setHeader(it->key, it->value);
		}
	}
	if (response.canSend() && !response.hasBody())
	{
		if (this->_error_page.count(response.getStatus())) {
			std::string fullpath = resolve(this->_root.nearest_root, this->_error_page[response.getStatus()]);
			Logger::debug("No response body sending error page at " + fullpath);
			response.sendFile(fullpath);
		} else {
			Logger::debug("No response body sending default response");
			response.sendDefault();
		}
	}
	response.end();
}

void	Router::route(Request &request, Response &response)
{
	if (this->handleRoutes(request, response)) {
		this->sendResponse(response);
	}
}

bool	Router::handleRoutes(Request &request, Response &response)
{
	/**
	 * Avant de faire quelque logique que ce soit, on s'assure que la réponse n'a pas déjà été
	 * envoyée pour une quelconque raison.
	 */
	if (!response.canSend())
		return (false);

	/**
	 * Selon Nginx, si la directive `client_max_body_size` a une valeur de 0, cela
	 * désactive la vérification de la limite de taille du corps de la requête.
	 */
	if (this->_client_body.size > 0) {
		if (request.getBody().size() > this->_client_body.size) {
			response.status(413);
			return (true);
		}
	}
	
	/**
	 * Nginx exécute la directive `return` avant toutes les autres.
	 * Si le code de retour est de type 3xx (redirection), alors la redirection est effectuée, sinon
	 * le code de retour est envoyé avec le corps de la réponse.
	 */
	if (this->_redirection.enabled) {
		if (this->_redirection.path.empty() && this->_redirection.data.empty()) {
			response.status(this->_redirection.status);
			return (true);
		}
		if (this->_redirection.status % 300 < 100) {
			Logger::debug("Redirect to: " + this->_redirection.path);
			response.redirect(this->_redirection.path, this->_redirection.status);
		} else {
			response.status(this->_redirection.status).send(this->_redirection.data);
		}
		return (true);
	}
	
	/**
	 * Si le router est configuré pour utiliser CGI, on execute le script CGI
	 * et on envoie la réponse.
	 */
	if (this->_cgi.enabled) {
		this->handleCGI(request, response);
		return (true);
	}
	this->call(request.getMethod(), request, response);
	return (true);
}

void	Router::handleGETMethod(Request &request, Response &response)
{
	Logger::debug("<------------ " B_BLUE "GET" B_GREEN " handler" RESET " ------------>");

	std::string fullpath = this->getLocalFilePath(request.getPath());
	if (!fullpath.size()) {
		response.status(500).end();
		return ;
	}
	Logger::debug("Full local path: " + fullpath);

	if (isDirectory(fullpath)) {
		std::string file_p = fullpath + "/"; 
		for (std::vector<std::string>::iterator it = this->_index.begin(); it != this->_index.end(); it++) {
			if (isFile(file_p + *it)) {
				response.sendFile(file_p + *it);
				return ;
			}
		}
		file_p += "index.html";
		if (isFile(file_p)) {
			response.sendFile(file_p);	
		} else {
			if (this->_autoindex) {
				response.setHeader("Content-Type", "text/html; charset=utf-8");
				response.send(this->getDirList(fullpath, request.getPath()));
			} else if (!this->_error_page.count(404)) {
				response.sendNotFound();
			} else {
				response.status(404);
			}
		}
	} else if (isFile(fullpath)) {
		response.sendFile(fullpath);
	} else if (!this->_error_page.count(404)) {
		response.sendNotFound();
	} else {
		response.status(404);
	}
}

void	Router::handleHEADMethod(Request &request, Response &response)
{
	Logger::debug("<------------ " B_BLUE "HEAD" B_GREEN " handler" RESET " ------------>");
	this->handleGETMethod(request, response);
	response.clearBody();
}

void	Router::handlePOSTMethod(Request &request, Response &response)
{
	Logger::debug("<------------ " B_BLUE "POST" B_GREEN " handler" RESET " ------------>");

	std::string fullpath = this->getLocalFilePath(request.getPath());
	if (!fullpath.size()) {
		response.status(500).end();
		return ;
	}
	Logger::debug("Full local path: " + fullpath);

	if (isFile(fullpath)) {
		if (appendFile(fullpath, request.getBody())) {
			response.status(500);
			return ;
		}
		response.status(200);
	} else {
		if (isDirectory(fullpath)) {
			response.status(409);
			return ;
		}
		if (createFile(fullpath, request.getBody())) {
			response.status(500);
			return ;
		}
		response.status(201);
	}
}

void	Router::handlePUTMethod(Request &request, Response &response)
{
	Logger::debug("<------------ " B_BLUE "PUT" B_GREEN " handler" RESET " ------------>");

	std::string fullpath = this->getLocalFilePath(request.getPath());
	if (!fullpath.size()) {
		response.status(500).end();
		return ;
	}
	Logger::debug("Full local path: " + fullpath);

	if (isFile(fullpath)) {
		if (deleteFile(fullpath)) {
			response.status(500);
			return ;
		}
		if (createFile(fullpath, request.getBody())) {
			response.status(500);
			return ;
		}
		response.status(204);
	} else {
		if (isDirectory(fullpath)) {
			response.status(409);
			return ;
		}
		if (createFile(fullpath, request.getBody())) {
			response.status(500);
			return ;
		}
		response.status(201);
	}
}

void	Router::handleDELETEMethod(Request &request, Response &response)
{
	Logger::debug("<------------ " B_BLUE "DELETE" B_GREEN " handler" RESET " ------------>");

	std::string fullpath = this->getLocalFilePath(request.getPath());
	if (!fullpath.size()) {
		response.status(500).end();
		return ;
	}
	Logger::debug("Full local path: " + fullpath);

	if (isFile(fullpath)) {
		if (deleteFile(fullpath)) {
			response.status(500);
			return ;
		}
		response.status(204);
	} else {
		if (isDirectory(fullpath)) {
			response.status(409);
		} else {
			response.status(404);
		}
	}
}

void	Router::handleTRACEMethod(Request &request, Response &response)
{
	Logger::debug("<------------ " B_BLUE "TRACE" B_GREEN " handler" RESET " ------------>");
	
	std::string res = request.getMethod() + " " + request.getPath() + " " + request.getVersion() + WBS_CRLF;
	wbs_mapss_t headers = request.getHeaders();
	for (wbs_mapss_t::const_iterator it = headers.begin(); it != headers.end(); it++) {
		res += it->first + ": " + it->second + WBS_CRLF;
	}
	response.status(200).send(res).end();
}

void	Router::handleCGI(Request &request, Response &response)
{
	Logger::debug("<------------ " B_BLUE "CGI" B_GREEN " handler" RESET " ------------>");

	std::string	body = request.getBody();
	std::string fullpath;

	if (this->_cgi.path.empty()) {
		Logger::error("router error: CGI path is empty");
		response.status(500).end();
		return ;
	}

	/**
	 * La méthode GET a un comportement légèrement différent des autres. Elle
	 * envoie le contenu du fichier demandé par la requête au CGI, tandis que les autres
	 * méthodes envoient directement le corps de la requête.
	 */
	if (request.getMethod() == "GET") {
		fullpath = this->getLocalFilePath(request.getPath());
		if (!fullpath.size()) {
			response.status(500).end();
			return ;
		}
		Logger::debug("Full local path: " + fullpath);
		
		if (!isFile(fullpath)) {
			response.status(404).end();
			return ;
		}

		std::ofstream		file;
		std::stringstream	buffer;

		file.open(fullpath.c_str(), std::ifstream::in);
		if (file.is_open() == false)
		{
			response.status(500).end();
			return ;
		}
		buffer << file.rdbuf();
		file.close();
		body = buffer.str();
	}
	response.sendCGI(CGIWorker::run(request, fullpath, this->_cgi.params, this->_cgi.path, body)).end();
}

void	Router::call(std::string method, Request &request, Response &response)
{
	(this->*Router::_method_handlers[method])(request, response);
}

bool Router::matchRoute(const std::string& route, Response &response) const
{
	regex_t	regex;
	int		result = 0;
	int		flags = REG_EXTENDED;

	std::string regexPattern = "";
	// Construire l'expression régulière en fonction du chemin de localisation et du modificateur
	if (this->_location.modifier == "=") {
		regexPattern = "^" + this->_location.path + "$"; // Exact Match
	} else if (this->_location.modifier == "~" || this->_location.modifier == "~*") {
		regexPattern = this->_location.path; // Regular Expression Case-Sensitive Match
		if (this->_location.modifier == "~*")
			flags |= REG_ICASE; // Regular Expression Case-Insensitive Match
	} else if (this->_location.modifier == "^~" || !this->_location.modifier.size()) {
		regexPattern = "^" + this->_location.path; // Prefix Match
	}

	Logger::debug("Trying location pattern: " + regexPattern);

	// Compiler l'expression régulière
	result = regcomp(&regex, regexPattern.c_str(), flags);

	if (result != 0) {
		Logger::error("router error: Regex compilation failed");
		response.status(500).end();
		return (false);
	}

	// Vérifier si la route correspond à l'expression régulière
	result = regexec(&regex, route.c_str(), 0, NULL, 0);

	// Libérer la mémoire utilisée par l'expression régulière compilée
	regfree(&regex);

	// Si la route correspond, retourner true, sinon retourner false
	return (result == 0);
}

std::string	cutLastSlash(const std::string &path)
{
	std::size_t lastSlashPos = path.find_last_of("/");
	std::string fileName = path.substr(lastSlashPos);
	std::size_t dotPos = fileName.find_last_of(".");

	if (dotPos == std::string::npos) {
		return ("");
	}
	return (fileName);
}

std::string	Router::getLocalFilePath(const std::string &requestPath)
{
	// La directive root consiste simplement à ajouter le chemin de la requête à la directive root
	if (!this->_root.isAlias) {
		if (this->_root.path == "/")
			return (requestPath);
		return (this->_root.path + requestPath);
	}

	std::string relativePath = requestPath;

	// Vérifier si le modificateur est une expression régulière
	if (this->_location.modifier == "~" || this->_location.modifier == "~*") {
		regex_t regex;
		int result;

		// Dans le cas ou le chemin de localisation est une expression régulière, l'expression 
		// régulière peut conserner des fichiers, dans ce cas on sauvegarde le dernier segment du chemin
		// de la requête pour le rajouter après la correspondance.
		std::string savedBlock = cutLastSlash(requestPath);

		int flags = REG_EXTENDED;

		// Ajouter le flag REG_ICASE si le modificateur est ~* (Case-Insensitive Match)
		if (this->_location.modifier == "~*") {
			flags |= REG_ICASE;
		}

		// Compiler l'expression régulière du modificateur
		result = regcomp(&regex, this->_location.path.c_str(), flags);
		if (result != 0) {
			Logger::error("router error: Regex compilation failed");
			return ("");
		}

		// Chercher la correspondance dans le chemin de la requête
		regmatch_t match;

		result = regexec(&regex, requestPath.c_str(), 1, &match, 0);
		if (result != 0)
			return ("");

		relativePath = resolve(requestPath.substr(match.rm_eo), savedBlock);

		// Libérer la mémoire utilisée par l'expression régulière compilée
		regfree(&regex);
	} else {
		// Si ce n'est pas une expression régulière, extraire simplement la partie de chemin après this->_location.path
		if (this->_location.path != "/")
			relativePath = requestPath.substr(this->_location.path.size());
	}

	return (resolve(this->_root.path, relativePath));
}

std::string	&Router::checkLeadingTrailingSlash(std::string &str)
{
	/**
	 * Si le chemin du router est '/' on ne fait rien.
	 */
	if (str == "/")
		return (str);
	/**
	 * Si le chemin du router est vide on le remplace par '/'.
	 */
	if (str.empty()) {
		str = "/";
		return (str);
	}
	/**
	 * Nginx n'a pas de comportement spécifique dépendant de la présence ou
	 * non du '/' au début du chemin de la route. Les chemins 'chemin' et '/chemin'
	 * ont le même comportement.
	 * Pour simplifier la suite, nous l'ajoutons s'il est manquant.
	 */
	if (str[0] != '/') {
		str.insert(0, "/");
	}
	/**
	 * La présence du '/' à la fin influe sur le comportement si le routeur est
	 * configuré comme strict, les chemins '/chemin' et '/chemin/' n'ont pas le même
	 * comportement dans ce cas.
	 * Si le routeur n'est pas strict, nous le supprimons s'il est présent pour
	 * simplifier la suite.
	 */
	if (str[str.size() - 1] == '/' && !this->_location.strict) {
		str.resize(str.size() - 1);
	}
	return (str);
}

bool	Router::isDefault(void) const
{
	return (this->_parent == NULL);
}

Router		*Router::getParent(void) const
{
	return (this->_parent);
}

const struct wbs_router_location	&Router::getLocation(void) const
{
	return (this->_location);
}

const std::string	&Router::getRoot(void) const
{
	return (this->_root.path);
}

const struct wbs_router_root	&Router::getRootData(void) const
{
	return (this->_root);
}

const struct wbs_router_redirection	&Router::getRedirection(void) const
{
	return (this->_redirection);
}

const std::vector<wbs_router_header_t>	&Router::getHeaders(void) const
{
	return (this->_headers.list);
}

const std::string	&Router::getErrorPage(const int status) const
{
	return (this->_error_page.at(status));
}

bool	Router::hasErrorPage(const int code) const
{
	return (this->_error_page.count(code) > 0);
}

size_t	Router::getClientMaxBodySize(void) const
{
	return (this->_client_body.size);
}

bool	Router::hasClientMaxBodySize(void) const
{
	return (this->_client_body.set);
}

const std::string	&Router::getCGIPath(void) const
{
	return (this->_cgi.path);
}

bool	Router::isProxy(void) const
{
	return (this->_proxy.enabled);
}

const struct wbs_router_proxy	&Router::getProxyConfig(void) const
{
	return (this->_proxy);
}

const struct wbs_router_timeout	&Router::getTimeout() const
{
	return (this->_timeout);
}

void	Router::reloadChildren(void)
{
	for (std::vector<Router *>::iterator it = this->_routes.begin(); it != this->_routes.end(); it++) {
		(*it)->reload();
	}
}

void	Router::reload(void)
{
	if (!this->_parent)
		return ;

	/**
	 * Cette fonction sert à mettre à jour les données héritées des niveaux de configuration précédents (des routeurs parents).
	 */
	if (!this->_root.set) {
		this->_root = this->_parent->getRootData();
	}
	if (!this->_root.set || (this->_root.set && this->_root.isAlias)) {
		this->_root.nearest_root = this->_parent->getRootData().nearest_root;
	}
	if (!this->_headers.enabled) {
		this->_headers.list = this->_parent->getHeaders();
	}
	if (!this->_allowed_methods.enabled) {
		this->_allowed_methods.methods = this->_parent->_allowed_methods.methods;
	}
	for (wbs_mapis_t::iterator it = this->_parent->_error_page.begin(); it != this->_parent->_error_page.end(); it++) {
		if (!this->_error_page.count(it->first)) {
			this->_error_page[it->first] = it->second;
		}
	}
	if (!this->_index.size()) {
		this->_index = this->_parent->_index;
	}
	if (this->_parent->hasClientMaxBodySize() && this->_client_body.size > this->_parent->getClientMaxBodySize()) {
		this->_client_body = this->_parent->_client_body;
	} else if (!this->_client_body.set) {
		this->_client_body = this->_parent->_client_body;
	}
	if (!this->_timeout.header_set) {
		this->_timeout.header_timeout = this->_parent->getTimeout().header_timeout;
	}
	if (!this->_timeout.body_set) {
		this->_timeout.body_timeout = this->_parent->getTimeout().body_timeout;
	}
	if (this->_proxy.enabled) {
		if (this->_proxy.protocol.empty()) {
			this->_proxy.protocol = this->_parent->getProxyConfig().protocol;
		}
		if (this->_proxy.host.empty()) {
			this->_proxy.host = this->_parent->getProxyConfig().host;
		}
		if (this->_proxy.port == 0) {
			this->_proxy.port = this->_parent->getProxyConfig().port;
		}
		if (this->_proxy.path.empty()) {
			this->_proxy.path = this->_parent->getProxyConfig().path;
		}
		if (this->_proxy.method.empty()) {
			this->_proxy.method = this->_parent->getProxyConfig().method;
		}
		for (wbs_mapss_t::const_iterator it = this->_parent->getProxyConfig().headers.begin(); it != this->_parent->getProxyConfig().headers.end(); it++) {
			if (!this->_proxy.headers.count(it->first)) {
				this->_proxy.headers[it->first] = it->second;
			}
		}
		for (std::vector<std::string>::const_iterator it = this->_parent->getProxyConfig().forwared.begin(); it != this->_parent->getProxyConfig().forwared.end(); it++) {
			if (!contains(this->_proxy.forwared, *it)) {
				this->_proxy.forwared.push_back(*it);
			}
		}
		for (std::vector<std::string>::const_iterator it = this->_parent->getProxyConfig().hidden.begin(); it != this->_parent->getProxyConfig().hidden.end(); it++) {
			if (!contains(this->_proxy.hidden, *it)) {
				this->_proxy.hidden.push_back(*it);
			}
		}
		/**
		 * FIXME:
		 * Fix inheritance of proxy forward headers/body
		 */
	}
	this->reloadChildren();
}

const std::string	Router::getDirList(const std::string &dirpath, std::string reqPath)
{
	wbs_mapss_t	content;
	std::string	res;

	this->checkLeadingTrailingSlash(reqPath);
	if (reqPath != "/")
		content.insert(content.begin(), std::pair<std::string, std::string>("..", reqPath));
	listFilesInDirectory(dirpath, content, false);
	res = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><title>listing directory <%= dir_name %></title><style>*{margin:0;padding:0;outline:0}body{padding:80px 100px;font:13px \"Helvetica Neue\",\"Lucida Grande\",Arial;background:#ece9e9 -webkit-gradient(linear,0 0,0 100%,from(#fff),to(#ece9e9));background:#ece9e9 -moz-linear-gradient(top,#fff,#ece9e9);background-repeat:no-repeat;color:#555;-webkit-font-smoothing:antialiased}h1,h2,h3{font-size:22px;color:#343434}h1 em,h2 em{padding:0 5px;font-weight:400}h1{font-size:60px}h2{margin-top:10px}h3{margin:5px 0 10px 0;padding-bottom:5px;border-bottom:1px solid #eee;font-size:18px}ul li{list-style:none}ul li:hover{cursor:pointer;color:#2e2e2e}ul li .path{padding-left:5px;font-weight:700}ul li .line{padding-right:5px;font-style:italic}ul li:first-child .path{padding-left:0}p{line-height:1.5}a{color:#555;text-decoration:none}a:hover{color:#303030}#stacktrace{margin-top:15px}.directory h1{margin-bottom:15px;font-size:18px}ul#files{width:100%;height:100%;overflow:hidden}ul#files li{float:left;width:30%;line-height:25px;margin:1px}ul#files li a{display:block;height:25px;border:1px solid transparent;-webkit-border-radius:5px;-moz-border-radius:5px;border-radius:5px;overflow:hidden;white-space:nowrap}ul#files li a:focus,ul#files li a:hover{background:rgba(255,255,255,.65);border:1px solid #ececec}ul#files li a.highlight{-webkit-transition:background .4s ease-in-out;background:#ffff4f;border-color:#e9dc51}#search{display:block;position:fixed;top:20px;right:20px;width:90px;-webkit-transition:width ease .2s,opacity ease .4s;-moz-transition:width ease .2s,opacity ease .4s;-webkit-border-radius:32px;-moz-border-radius:32px;-webkit-box-shadow:inset 0 0 3px rgba(0,0,0,.25),inset 0 1px 3px rgba(0,0,0,.7),0 1px 0 rgba(255,255,255,.03);-moz-box-shadow:inset 0 0 3px rgba(0,0,0,.25),inset 0 1px 3px rgba(0,0,0,.7),0 1px 0 rgba(255,255,255,.03);-webkit-font-smoothing:antialiased;text-align:left;font:13px \"Helvetica Neue\",Arial,sans-serif;padding:4px 10px;border:none;background:0 0;margin-bottom:0;outline:0;opacity:.7;color:#888}#search:focus{width:120px;opacity:1}#files span{display:inline-block;overflow:hidden;text-overflow:ellipsis;text-indent:10px}#files .name{background-repeat:no-repeat}#files .icon .name{text-indent:28px}.view-tiles .name{width:100%;background-position:8px 5px}.view-tiles .date,.view-tiles .size{display:none}ul#files.view-details li{float:none;display:block;width:90%}ul#files.view-details li.header{height:25px;background:#000;color:#fff;font-weight:700}.view-details .header{border-radius:5px}.view-details .name{width:60%;background-position:8px 5px}.view-details .size{width:10%}.view-details .date{width:30%}.view-details .date,.view-details .size{text-align:right;direction:rtl}@media (max-width:768px){body{font-size:13px;line-height:16px;padding:0}#search{position:static;width:100%;font-size:2em;line-height:1.8em;text-indent:10px;border:0;border-radius:0;padding:10px 0;margin:0}#search:focus{width:100%;border:0;opacity:1}.directory h1{font-size:2em;line-height:1.5em;color:#fff;background:#000;padding:15px 10px;margin:0}ul#files{border-top:1px solid #cacaca}ul#files li{float:none;width:auto!important;display:block;border-bottom:1px solid #cacaca;font-size:2em;line-height:1.2em;text-indent:0;margin:0}ul#files li:nth-child(odd){background:#e0e0e0}ul#files li a{height:auto;border:0;border-radius:0;padding:15px 10px}ul#files li a:focus,ul#files li a:hover{border:0}#files .date,#files .header,#files .size{display:none!important}#files .name{float:none;display:inline-block;width:100%;text-indent:0;background-position:0 50%}#files .icon .name{text-indent:41px}}#files .icon-directory .name{background-image:url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAMAAAAoLQ9TAAAABGdBTUEAALGPC/xhBQAAAWtQTFRFAAAA/PPQ9Nhc2q402qQ12qs2/PTX2pg12p81+/LM89NE9dto2q82+/fp2rM22qY39d6U+/bo2qo2/frx/vz32q812qs12qE279SU8c4w9NZP+/LK//367s9y7s925cp0/vzw9t92//342po2/vz25s1579B6+OSO2bQ0/v799NyT8tE79dld8Msm+OrC/vzx79KA2IYs7s6I9d6R4cJe9+OF/PLI/fry79OF/v30//328tWB89RJ8c9p8c0u9eCf//7+9txs6sts5Mdr+++5+u2z/vrv+/fq6cFz8dBs8tA57cpq+OaU9uGs27Y8//799NdX/PbY9uB89unJ//z14sNf+emh+emk+vDc+uys9+OL8dJy89NH+eic8tN5+OaV+OWR9N2n9dtl9t529+KF9+GB9Nue9NdU8tR/9t5y89qW9dpj89iO89eG/vvu2pQ12Y4z/vzy2Ict/vvv48dr/vzz4sNg///+2Igty3PqwQAAAAF0Uk5TAEDm2GYAAACtSURBVBjTY2AgA2iYlJWVhfohBPg0yx38y92dS0pKVOVBAqIi6sb2vsWWpfrFeTI8QAEhYQEta28nCwM1OVleZqCAmKCEkUdwYWmhQnFeOStQgL9cySqkNNDHVJGbiY0FKCCuYuYSGRsV5KgjxcXIARRQNncNj09JTgqw0ZbkZAcK5LuFJaRmZqfHeNnpSucDBQoiEtOycnIz4qI9bfUKQA6pKKqAgqIKQyK8BgAZ5yfODmnHrQAAAABJRU5ErkJggg==)}#files .icon-text .name {background-image: url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAQAAAC1+jfqAAAABGdBTUEAAK/INwWK6QAAABl0RVh0U29mdHdhcmUAQWRvYmUgSW1hZ2VSZWFkeXHJZTwAAADoSURBVBgZBcExblNBGAbA2ceegTRBuIKOgiihSZNTcC5LUHAihNJR0kGKCDcYJY6D3/77MdOinTvzAgCw8ysThIvn/VojIyMjIyPP+bS1sUQIV2s95pBDDvmbP/mdkft83tpYguZq5Jh/OeaYh+yzy8hTHvNlaxNNczm+la9OTlar1UdA/+C2A4trRCnD3jS8BB1obq2Gk6GU6QbQAS4BUaYSQAf4bhhKKTFdAzrAOwAxEUAH+KEM01SY3gM6wBsEAQB0gJ+maZoC3gI6iPYaAIBJsiRmHU0AALOeFC3aK2cWAACUXe7+AwO0lc9eTHYTAAAAAElFTkSuQmCC);}</style></head><body class=\"directory\"><div id=\"wrapper\"><h1><%= dir_tree %></h1><ul id=\"files\" class=\"view-tiles\"> <%= dir_content %> </ul></div></body></html>";
	replace(res, "<%= dir_name %>", reqPath);
	std::vector<std::string> list = split(reqPath, '/');
	std::string temp = "/";
	for (std::vector<std::string>::iterator it = list.begin(); it != list.end(); it++) {
		if (it > list.begin() + 1)
			temp += "/";
		temp += *it;
		*it = "<a href=\""+temp+"\">"+*it+"</a>";
	}
	std::string dirhierachy = list.size() == 1 ? "/" : join(list, " / ");
	replace(res, "<%= dir_tree %>", dirhierachy);
	temp = "";
	std::string icon;
	if (reqPath == "/")
		reqPath.clear();
	for (wbs_mapss_t::iterator it = content.begin(); it != content.end(); it++) {
		if ((it->first == "..") || isDirectory(it->second))
			icon = "icon-directory";
		else
			icon = "icon-text";
		temp += "<li><a href=\""+reqPath+"/"+it->first+"\" class=\"icon "+icon+"\" title=\""+it->first+"\"><span class=\"name\">"+it->first+"</span><span class=\"size\"></span><span class=\"date\"></span></a></li>";
	}
	replace(res, "<%= dir_content %>", temp);
	return (res);
}

void	Router::print(std::ostream &os) const
{
	std::string space = std::string(this->level + this->isDefault(), '\t');

	os << space << B_GREEN"Router: " << RESET;
	if (this->_parent && !this->_parent->isDefault())
		os << "parent: " << this->_parent->getLocation().path;
	os << "\n";
	os << space << B_CYAN"Path: " << RESET << (this->_location.modifier.size() ? this->_location.modifier+" " : "") << this->_location.path << "\n";
	os << space << B_CYAN"Methods:" << RESET;
	if (this->_allowed_methods.methods.empty())
		os << " all";
	for (std::vector<std::string>::const_iterator it = this->_allowed_methods.methods.begin(); it != this->_allowed_methods.methods.end(); it++)
		os << " " << *it;
	os << "\n";
	os << space << B_CYAN"Root: " << RESET << this->_root.path << (this->_root.isAlias ? " (alias)" : "") << "\n";
	os << space << B_CYAN"Nearest Root: " << RESET << this->_root.nearest_root << "\n";
	os << space << B_CYAN"Redirection: " << RESET << (this->_redirection.enabled ? this->_redirection.path + " status: " + toString(this->_redirection.status) + (this->_redirection.data.size() ? " data: " + this->_redirection.data : "") : "none") << "\n";
	os << space << B_CYAN"Autoindex: " << RESET << (this->_autoindex ? "enabled" : "disabled") << "\n";
	os << space << B_CYAN"Index: " << RESET;
	for (std::vector<std::string>::const_iterator it = this->_index.begin(); it != this->_index.end(); it++)
		os << *it << " ";
	os << "\n";
	os << space << B_CYAN"CGI: " << RESET << (this->_cgi.enabled ? "enabled" : "disabled") << "\n";
	if (this->_cgi.enabled) {
		os << space << B_CYAN"CGI path: " << RESET << this->_cgi.path << "\n";
	}
	os << space << B_CYAN"Proxy: " << RESET << (this->_proxy.enabled ? "enabled" : "disabled") << "\n";
	if (this->_proxy.enabled) {
		os << space << B_CYAN"Proxy protocol: " << RESET << this->_proxy.protocol << "\n";
		os << space << B_CYAN"Proxy host: " << RESET << this->_proxy.host << "\n";
		os << space << B_CYAN"Proxy port: " << RESET << this->_proxy.port << "\n";
		os << space << B_CYAN"Proxy method: " << RESET << (this->_proxy.method.length() > 0 ? this->_proxy.method : "-")<< "\n";
		os << space << B_CYAN"Proxy uri: " << RESET << this->_proxy.path << "\n";
		os << space << B_CYAN"Proxy forward body: " << RESET << (this->_proxy.forward_body ? "enabled" : "disabled") << "\n";
		os << space << B_CYAN"Proxy forward headers: " << RESET << (this->_proxy.forward_headers ? "enabled" : "disabled") << "\n";
		os << space << B_CYAN"Proxy set headers: " << RESET << "\n";
		for (wbs_mapss_t::const_iterator it = this->_proxy.headers.begin(); it != this->_proxy.headers.end(); it++)
			os << space << it->first << ": " << it->second << "\n";
		os << space << B_CYAN"Proxy forwarded headers: " << RESET;
		for (std::vector<std::string>::const_iterator it = this->_proxy.forwared.begin(); it != this->_proxy.forwared.end(); it++)
			os << *it << " ";
		os << "\n";
		os << space << B_CYAN"Proxy hidden headers: " << RESET;
		for (std::vector<std::string>::const_iterator it = this->_proxy.hidden.begin(); it != this->_proxy.hidden.end(); it++)
			os << *it << " ";
		os << "\n";
	}
	os << space << B_CYAN"Client max body size: " << RESET << getSize(this->_client_body.size) << "\n";
	os << space << B_CYAN"Header timeout: " << RESET << getTime(this->_timeout.header_timeout) << "\n";
	os << space << B_CYAN"Body timeout: " << RESET << getTime(this->_timeout.body_timeout) << "\n";
	if (this->_headers.list.size()) {
		os << space << B_CYAN"Response headers: " << RESET << "\n";
		for (std::vector<struct wbs_router_headers::wbs_router_header>::const_iterator it = this->_headers.list.begin(); it != this->_headers.list.end(); it++)
			os << space << it->key << ": " << it->value << (it->always ? " (always)" : "") << "\n";
	}
	os << space << B_CYAN"Error pages: " << RESET << "\n";
	for (wbs_mapis_t::const_iterator it = this->_error_page.begin(); it != this->_error_page.end(); it++)
		os << space << it->first << " => " << it->second << "\n";
	if (this->_routes.size()) {
		os << "\n";
		if (this->isDefault())
			os << std::string(this->level, '\t') << B_ORANGE << "Routers: " << RESET;
		else
			os << space << B_ORANGE << "Sub-Routers of " << this->_location.path << ": " << RESET;
		os << "\n";
		for (std::vector<Router *>::const_iterator it = this->_routes.begin(); it != this->_routes.end(); it++) {
			os << **it;
		}
	}
}
