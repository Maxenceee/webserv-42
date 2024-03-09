/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:05:17 by mgama             #+#    #+#             */
/*   Updated: 2024/03/09 02:07:15 by mgama            ###   ########.fr       */
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
	map["HEAD"] 	= &Router::handleHEADMethod;
	map["POST"] 	= &Router::handlePOSTMethod;
	map["PUT"]		= &Router::handlePUTMethod;
	map["DELETE"]	= &Router::handleDELETEMethod;
	map["TRACE"]	= &Router::handleTRACEMethod;
	return (map);
}

Router::Router(Router *parent, const struct s_Router_Location location): _parent(parent), _location(location), _autoindex(false)
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
	 * changées en appellant la méthode Router::addHeader().
	 */
	if (parent) {
		this->_headers.enabled = false;
		this->_headers.list = parent->getHeaders();
	}

	/**
	 * Par défaut le router hérite des pages d'erreur de son parent.
	 */
	if (parent) {
		this->_error_page = parent->_error_page;
	}
	
	/**
	 * Nginx definit la valeur par defaut comme étant 1Mo.
	 */
	if (parent) {
		this->_client_body.size = parent->getClientMaxBodySize();
	} else {
		this->_client_body.size = 1024 * 1024; // 1MB
	}

	this->_client_body.set = false;
	this->_redirection.enabled = false;
	this->_cgi.enabled = false;
}

Router::~Router(void)
{
	for (std::vector<Router *>::iterator it = this->_routes.begin(); it != this->_routes.end(); it++)
		delete *it;
}

const bool	Router::isDefault(void) const
{
	return (this->_parent == NULL);
}

Router		*Router::getParent(void) const
{
	return (this->_parent);
}

void	Router::allowMethod(const std::string method)
{
	/**
	 * Router::allowMethod() indique au router qu'elle méthode HTTP il doit servir. Si aucune méthode
	 * n'est spécifiée le router les accepte toutes.
	 */
	if (Server::isValidMethod(method))
		this->_allowed_methods.push_back(method);
	else
		Logger::error("router error: Invalid method found. No such `" + method + "`");
}

void	Router::allowMethod(const std::vector<std::string> method)
{
	for (std::vector<std::string>::const_iterator it = method.begin(); it != method.end(); it++)
		this->allowMethod(*it);
}

void	Router::setRoot(const std::string path)
{
	/**
	 * Cette fonction indique au router son dossier racine, dossier à partir duquel il servira le contenu.
	 * Par défaut il l'hérite de son parent.
	 * Pour simplifier, le chemin de la requête est ajouté à la fin du chemin de `root` pour former le
	 * chemin vers la ressource.
	 * (ex: router = /static, request = /static/chemin/vers, root = ./public, chemin final => ./public/chemin/vers)
	 * 
	 * Attention, dans Nginx la directive `alias` a la priorité sur `root`, si `alias` est définie cette
	 * directive sera écrasée.
	 */
	if (this->_root.isAlias && this->_root.set) {
		Logger::info("router info: Aliasing is already enbaled for this router.");
	} else if (this->_root.set) {
		Logger::info("router info: Root is already set, abording.");
	} else if (!isDirectory(path)) {
		throw std::invalid_argument("router error: Not a directory: "+path);
	} else {
		this->_root.set = true;
		this->_root.path = path;
		this->_root.nearest_root = this->_root.path;
		this->_root.isAlias = false;
		this->reloadChildren();
	}
}

void	Router::setAlias(const std::string path)
{
	/**
	 * Permet de définir la directive `alias` pour ce router. Contrairement à `root`,
	 * la directive `alias` remplace le segment de l'URL correspondant par le chemin spécifié.
	 * (ex: router = /images, request = /images/photo.jpg, alias = ./public/photos, chemin final => ./public/photos/photo.jpg)
	 * (dans ce cas root aurait donné: router = /images, request = /images/photo.jpg, root = ./public/photos, chemin final => ./public/photos/images/photo.jpg)
	 * 
	 * Attention, dans Nginx la directive `alias` a la priorité sur `root`, si `root` a été définie
	 * précédemment cette dernière sera écrasée.
	 */
	if (!this->_root.isAlias && this->_root.set) {
		Logger::info("router info: Root is already set, abording.");
	} else if (!isDirectory(path)) {
		throw std::invalid_argument("router error: Not a directory: "+path);
	} else {
		if (this->_root.set)
			Logger::info("router info: Overriding `root` directive.");
		this->_root.set = true;
		this->_root.path = path;
		this->_root.isAlias = true;
		this->reloadChildren();
	}
}

const struct s_Router_Location	&Router::getLocation(void) const
{
	return (this->_location);
}

const std::string	&Router::getRoot(void) const
{
	return (this->_root.path);
}

const struct s_Router_Root	&Router::getRootData(void) const
{
	return (this->_root);
}

void	Router::setRedirection(std::string to, int status)
{
	/**
	 * Définit le chemin de redirection du router, le statut par défaut est 302 (Found).
	 * Si le chemin est vide alors le statut est retourné sans redirection.
	 */
	this->_redirection.path = to;
	this->_redirection.status = status;
	this->_redirection.enabled = true;
}

const struct s_Router_Redirection	&Router::getRedirection(void) const
{
	return (this->_redirection);
}

void	Router::setAutoIndex(const bool autoindex)
{
	this->_autoindex = autoindex;
}

void	Router::setIndex(const std::vector<std::string> index)
{
	this->_index = index;
}

void	Router::addIndex(const std::string index)
{
	this->_index.push_back(index);
}

void	Router::addHeader(const std::string key, const std::string value, const bool always)
{
	/**
	 * Cette méthode permet d'ajouter des en-têtes à la réponse du router.
	 * Si `always` est vrai, l'en-tête sera ajouté quelque soit le code de réponse.
	 * Les en-têtes sont héritées des niveaux de configuration précédents si et seulement si
	 * aucun n'est défini au niveau de configuration actuel. 
	 */
	if (!this->_headers.enabled) {
		this->_headers.enabled = true;
		this->_headers.list.clear();
	}
	this->_headers.list.push_back((Router_Header_t){key, value, always});
	this->reloadChildren();
}

const std::vector<Router_Header_t>	&Router::getHeaders(void) const
{
	return (this->_headers.list);
}

void	Router::setErrorPage(const int code, const std::string path)
{
	if (this->_error_page.count(code)) {
		Logger::info("router info: overriding previous error page for " + toString<int>(code));
	}
	this->_error_page[code] = path;
	checkLeadingTrailingSlash(this->_error_page[code]);
	this->reloadChildren();
}

const std::string	&Router::getErrorPage(const int status) const
{
	return (this->_error_page.at(status));
}

const bool			Router::hasErrorPage(const int code) const
{
	return (this->_error_page.count(code) > 0);
}

void	Router::setClientMaxBodySize(const std::string &size)
{
	this->_client_body.size = parseSize(size);
	if (this->_client_body.size < 0) {
		throw std::invalid_argument("router error: Invalid size: "+size);
	}
	this->_client_body.set = true;
	this->reloadChildren();
}

void	Router::setClientMaxBodySize(const int size)
{
	this->_client_body.size = size;
	if (this->_client_body.size < 0) {
		throw std::invalid_argument("router error: Invalid size: "+toString<int>(size));
	}
	this->_client_body.set = true;
	this->reloadChildren();
}

const int	Router::getClientMaxBodySize(void) const
{
	return (this->_client_body.size);
}

void	Router::setCGI(const std::string path, const std::string extension)
{
	if (!isFile(path))
		throw std::invalid_argument("router error: Not a directory: "+path);
	this->_cgi.path = path;
	this->_cgi.enabled = true;
}

void	Router::enableCGI(void)
{
	this->_cgi.enabled = true;
}

void	Router::addCGIParam(const std::string key, const std::string value)
{
	this->_cgi.params[key] = value;
}

const std::string	&Router::getCGIPath(void) const
{
	return (this->_cgi.path);
}

void	Router::use(Router *router)
{
	/**
	 * Cette méthode permet d'ajouter un router enfant à ce router.
	 * Les routes du router enfant seront servies par ce router.
	 */
	this->_routes.push_back(router);
}

std::vector<Router *>	&Router::getRoutes(void)
{
	return (this->_routes);
}

void	Router::route(Request &request, Response &response)
{
	if (this->handleRoutes(request, response)) {
		if (response.canSend()) {
			for (std::vector<Router_Header_t>::iterator it = this->_headers.list.begin(); it != this->_headers.list.end(); it++) {
				if (response.canAddHeader() || it->always)
					response.setHeader(it->key, it->value);
			}
		}
		if (response.canSend() && !response.hasBody())
		{
			if (this->_error_page.count(response.getStatus())) {
				std::string fullpath = this->_root.nearest_root + this->_error_page[response.getStatus()];
				Logger::debug("router full path: " + fullpath);
				response.sendFile(fullpath);
			} else {
				Logger::debug("router default response");
				response.sendDefault();
			}
		}
		response.end();
	}
}

bool	Router::handleRoutes(Request &request, Response &response)
{
	/**
	 * Avant de faire quelque logique que ce soit on s'assure que la réponse n'a pas déjà été
	 * envoyé pour une quelconque raison.
	 */
	if (!response.canSend())
		return (false);
	/**
	 * On compare le chemin du router et celui de la requête.
	 */
	if (this->matchRoute(request.getPath(), response))
	{
		if (this->_allowed_methods.size() && !contains(this->_allowed_methods, request.getMethod()))
		{
			response.setHeader("Allow", Response::formatMethods(this->_allowed_methods));
			response.status(405);
			return (true);
		}
		/**
		 * Selon Nginx si la directive `client_max_body_size` a une valeur de 0 alors cela
		 * desactive la verification de la limite de taille du corps de la requête.
		 */
		if (this->_client_body.size > 0) {
			if (request.getBody().size() > this->_client_body.size) {
				response.status(413);
				return (true);
			}
		}
		
		/**
		 * Evaluation des routes enfants.
		 */
		Logger::debug("sub-router: " + toString<int>(this->_routes.size()));
		for (std::vector<Router *>::iterator it = this->_routes.begin(); it != this->_routes.end(); it++) {
			(*it)->route(request, response);
			if (!response.canSend())
				return (true);
		}
		
		/**
		 * Nginx n'est pas très clair quant aux priorités entre `root`/`alias` et
		 * `return`. Nous avons fait le choix de toujours donner la priorité à
		 * return.
		 */
		if (this->_redirection.enabled) {
			if (this->_redirection.path.empty()) {
				response.status(this->_redirection.status);
				return (true);
			}
			Logger::debug("redirect to: " + this->_redirection.path);
			response.redirect(this->_redirection.path, this->_redirection.status);
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
	return (false);
}

void	Router::handleGETMethod(Request &request, Response &response)
{
	Logger::debug("<------------ "B_BLUE"GET"B_GREEN" handler"RESET" ------------>");
	Logger::debug("requestPath: " + request.getPath());

	std::string fullpath = this->getLocalFilePath(request.getPath());
	if (!fullpath.size()) {
		response.status(500).end();
		return ;
	}
	Logger::debug("full path: " + fullpath);

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
			} else {
				response.sendNotFound();
			}
		}
	} else if (isFile(fullpath)) {
		response.sendFile(fullpath);
	} else {
		response.sendNotFound();
	}
}

void	Router::handleHEADMethod(Request &request, Response &response)
{
	Logger::debug("<------------ "B_BLUE"HEAD"B_GREEN" handler"RESET" ------------>");
	this->handleGETMethod(request, response);
	response.clearBody();
}

void	Router::handlePOSTMethod(Request &request, Response &response)
{
	Logger::debug("<------------ "B_BLUE"POST"B_GREEN" handler"RESET" ------------>");
	Logger::debug("post request: " + request.getBody());

	std::string fullpath = this->getLocalFilePath(request.getPath());
	if (!fullpath.size()) {
		response.status(500).end();
		return ;
	}
	Logger::debug("full path: " + fullpath);

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
	Logger::debug("<------------ "B_BLUE"PUT"B_GREEN" handler"RESET" ------------>");
	Logger::debug("put request: " + request.getBody());

	std::string fullpath = this->getLocalFilePath(request.getPath());
	if (!fullpath.size()) {
		response.status(500).end();
		return ;
	}
	Logger::debug("full path: " + fullpath);

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
	Logger::debug("<------------ "B_BLUE"DELETE"B_GREEN" handler"RESET" ------------>");

	std::string fullpath = this->getLocalFilePath(request.getPath());
	if (!fullpath.size()) {
		response.status(500).end();
		return ;
	}
	Logger::debug("full path: " + fullpath);

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
	Logger::debug("<------------ "B_BLUE"TRACE"B_GREEN" handler"RESET" ------------>");
	
	std::string res = request.getMethod() + " " + request.getPath() + " " + request.getVersion() + "\r\n";
	t_mapss headers = request.getHeaders();
	for (t_mapss::const_iterator it = headers.begin(); it != headers.end(); it++) {
		res += it->first + ": " + it->second + "\r\n";
	}
	response.status(200).send(res).end();
}

void	Router::handleCGI(Request &request, Response &response)
{
	Logger::debug("<------------ "B_BLUE"CGI"B_GREEN" handler"RESET" ------------>");

	std::string	body = request.getBody();

	if (this->_cgi.path.empty()) {
		Logger::error("router error: CGI path is empty");
		response.status(500).end();
		return ;
	}

	/**
	 * La méthode GET a un comportement légèrement différent des autres, elle
	 * envoie le contenu du fichier demandé par la requête au CGI. Les autres
	 * méthodes envoient directement le corps de la requête.
	 */
	if (request.getMethod() == "GET") {
		std::string fullpath = this->getLocalFilePath(request.getPath());
		if (!fullpath.size()) {
			response.status(500).end();
			return ;
		}
		Logger::debug("full path: " + fullpath);
		
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
	response.sendCGI(CGIWorker::run(request, this->_cgi.params, this->_cgi.path, body)).end();
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

	// if (this->_location.strict) {
		// Si ce n'est pas strict, ajouter .* à la fin pour correspondre à n'importe quelle fin
		// regexPattern += ".*";
	// }

	Logger::debug("location pattern: " + regexPattern);

	// Compiler l'expression régulière
	result = regcomp(&regex, regexPattern.c_str(), flags);

	if (result != 0) {
		Logger::error("Regex compilation failed");
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
			Logger::error("Regex compilation failed");
			return ("");
		}

		// Chercher la correspondance dans le chemin de la requête
		regmatch_t match;

		result = regexec(&regex, requestPath.c_str(), 1, &match, 0);
		if (result != 0)
			return ("");
		relativePath = requestPath.substr(match.rm_eo) + savedBlock;

		// Libérer la mémoire utilisée par l'expression régulière compilée
		regfree(&regex);
	} else {
		// Si ce n'est pas une expression régulière, extraire simplement la partie de chemin après this->_location.path
		if (this->_location.path != "/")
			relativePath = requestPath.substr(this->_location.path.size());
	}

	std::string fullPath = this->_root.path + relativePath;;

	return (fullPath);
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
	 * non du '/' au début du chemin de la route. Le chemins 'chemin' et '/chemin'
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
	if (str[str.size() - 1] == '/' && !this->_location.strict) {
		str.resize(str.size() - 1);
	}
	return (str);
}

void	Router::reloadChildren(void)
{
	for (std::vector<Router *>::iterator it = this->_routes.begin(); it != this->_routes.end(); it++) {
		(*it)->reload();
	}
}

void	Router::reload(void)
{
	/**
	 * Cette fonction sert à mettre à jour les données héritées des niveaux de configuration précédents (des routers parents).
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
	for (std::map<int, std::string>::iterator it = this->_parent->_error_page.begin(); it != this->_parent->_error_page.end(); it++) {
		if (!this->_error_page.count(it->first)) {
			this->_error_page[it->first] = it->second;
		}
	}
	if (!this->_client_body.set) {
		this->_client_body = this->_parent->_client_body;
	}
	this->reloadChildren();
}

const std::string	Router::getDirList(const std::string dirpath, std::string reqPath)
{
	t_mapss		content;
	std::string	res;

	this->checkLeadingTrailingSlash(reqPath);
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
	std::string dirhierachy = join(list, " / ");
	replace(res, "<%= dir_tree %>", dirhierachy);
	temp = "";
	std::string icon;
	for (t_mapss::iterator it = content.begin(); it != content.end(); it++) {
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
	os << "\t" << B_GREEN"Router: " << RESET;
	if (this->_parent && !this->_parent->isDefault())
		os << "parent: " << this->_parent->getLocation().path;
	os << "\n";
	os << "\t" << B_CYAN"Path: " << RESET << (this->_location.modifier.size() ? this->_location.modifier+" " : "") << this->_location.path << "\n";
	os << "\t" << B_CYAN"Methods:" << RESET;
	if (this->_allowed_methods.empty())
		os << " all";
	for (std::vector<std::string>::const_iterator it = this->_allowed_methods.begin(); it != this->_allowed_methods.end(); it++)
		os << " " << *it;
	os << "\n";
	os << "\t" << B_CYAN"Root: " << RESET << this->_root.path << (this->_root.isAlias ? " (alias)" : "") << "\n";
	os << "\t" << B_CYAN"Nearest Root: " << RESET << this->_root.nearest_root << "\n";
	os << "\t" << B_CYAN"Redirection: " << RESET << (this->_redirection.enabled ? this->_redirection.path + " status: " + toString(this->_redirection.status) : "none") << "\n";
	os << "\t" << B_CYAN"Autoindex: " << RESET << (this->_autoindex ? "enabled" : "disabled") << "\n";
	os << "\t" << B_CYAN"Index: " << RESET;
	for (std::vector<std::string>::const_iterator it = this->_index.begin(); it != this->_index.end(); it++)
		os << *it << " ";
	os << "\n";
	os << "\t" << B_CYAN"CGI: " << RESET << (this->_cgi.enabled ? "enabled" : "disabled") << "\n";
	if (this->_cgi.enabled) {
		os << "\t" << B_CYAN"CGI path: " << RESET << this->_cgi.path << "\n";
	}
	os << "\t" << B_CYAN"Client max body size: " << RESET << getSize(this->_client_body.size) << "\n";
	if (this->_headers.list.size()) {
		os << "\t" << B_CYAN"Response headers: " << RESET << "\n";
		for (std::vector<struct s_Router_Headers::s_Router_Header>::const_iterator it = this->_headers.list.begin(); it != this->_headers.list.end(); it++)
			os << "\t" << it->key << ": " << it->value << (it->always ? " (always)" : "") << "\n";
	}
	os << "\t" << B_CYAN"Error pages: " << RESET << "\n";
	for (std::map<int, std::string>::const_iterator it = this->_error_page.begin(); it != this->_error_page.end(); it++)
		os << "\t" << it->first << " => " << it->second << "\n";
	if (this->_routes.size()) {
		if (this->isDefault())
			os << B_ORANGE << "Routers: " << RESET;
		else
			os << B_ORANGE << "Sub-Routers of " << this->_location.path << ": " << RESET;
		os << "\n";
		for (std::vector<Router *>::const_iterator it = this->_routes.begin(); it != this->_routes.end(); it++) {
			os << **it;
		}
	}
}
