/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:05:17 by mgama             #+#    #+#             */
/*   Updated: 2024/01/15 18:44:48 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Router.hpp"

/**
 * TODO:
 * Gerer les pages d'erreurs (error_page) envoyer la page si necessaire pour envoyer celle du serveur
 */

Router::Router(Server &server, const std::string path, const bool strict): _server(server), _path(path), _strict(strict), _autoindex(false)
{
	/**
	 * Par défault le router hérite du chemin de son parent. Celui-ci peut être
	 * changé en appellant la méthode Router::setRoot ou Router::setAlias.
	 */
	this->_root.path = server.getDefaultHandler().getRoot();
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
	 * Router::allowMethod() indique au router qu'elle méthode HTTP il doit servir. Si aucune méthode
	 * n'est spécifiée le router les accepte toutes.
	 */
	if (this->isValidMethod(method))
		this->_allowed_methods.push_back(method);
	else
		std::cerr << B_RED"router error: Invalid method found. No such `" << method << "`" << RESET << std::endl;
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
		std::cout << B_YELLOW"router info: Aliasing is already enbaled for this router." << RESET << std::endl;
	} else if (this->_root.set) {
		std::cout << B_YELLOW"router info: Root is already set, abording." << RESET << std::endl;
	} else if (!isDirectory(path)) {
		throw std::invalid_argument(B_RED"router error: Not a directory: "+path+RESET);
	} else {
		this->_root.path = path;
		this->_root.isAlias = false;
	}
	listDirContent(path);
}

void	Router::setAlias(const std::string path)
{
	/**
	 * Permet de définir la directive `alias` pour ce router. Contrairement à `root`,
	 * la directive `alias` remplace le segment de l'URL correspondant par le chemin spécifié.
	 * (ex: router = /images, request = /images/photo.jpg, alias = ./public/images, chemin final => ./public/images/photo.jpg)
	 * (dans ce cas root aurait donné: router = /images, request = /images/photo.jpg, root = ./public/images, chemin final => ./public/images/images/photo.jpg)
	 * 
	 * Attention, dans Nginx la directive `alias` a la priorité sur `root`, si `root` a été définie
	 * précédemment cette dernière sera écrasée.
	 */
	if (!this->_root.isAlias && this->_root.set) {
		std::cout << B_YELLOW"router info: Alias is already set, abording." << RESET << std::endl;
	} else if (!isDirectory(path)) {
		throw std::invalid_argument(B_RED"router error: Not a directory: "+path+RESET);
	} else {
		if (this->_root.set)
			std::cout << B_YELLOW"router info: Overriding `root` directive." << RESET << std::endl;
		this->_root.path = path;
		this->_root.isAlias = true;
	}
	listDirContent(path);
}

const std::string	Router::getRoot(void) const
{
	return (this->_root.path);
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

void	Router::setAutoIndex(const bool autoindex)
{
	this->_autoindex = autoindex;
}

void	Router::setErrorPage(const int code, const std::string path)
{
	if (this->_error_page.count(code)) {
		std::cout << B_YELLOW"router info: overriding previous error page for " << code << RESET << std::endl;
	}
	this->_error_page[code] = path;
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
	 * Avant de faire quelque logique que ce soit on s'assure que la réponse n'a pas déjà été
	 * envoyé pour une quelconque raison.
	 */
	if (!response.canSend())
		return ;
	/**
	 * On compare le chemin du router et celui de la requête.
	 * Pour le moment toutes les requête sont considérées comme GET.
	 * TODO:
	 * Gérer correctement les chemins du router et les différentes méthodes. Seule la
	 * directive `root` est géré pour le moment, il manque la gestion de `alias`.
	 */
	if ((request.getPath().compare(0, this->_path.size(), this->_path) == 0 && !this->_strict) || (request.getPath() == this->_path))
	{
		/**
		 * Nginx n'est pas très clair quant au priorité entre `root`/`alias` et
		 * `return`. Nous avons fait le choix de toujours donner la priorité à
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
		/**
		 * TODO:
		 * Gérer la directive `index`
		 */
		if (isDirectory(fullpath)) {
			if (isFile(fullpath+"/index.html")) {
				response.sendFile(fullpath+"/index.html");	
			} else {
				std::cerr << "cannot get " << fullpath << std::endl;
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