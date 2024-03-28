/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:04:59 by mgama             #+#    #+#             */
/*   Updated: 2024/03/28 03:43:53 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "webserv.hpp"
#include "server/Server.hpp"
#include "server/ServerConfig.hpp"
#include "request/Request.hpp"
#include "response/Response.hpp"
#include "cgi/CGIWorker.hpp"

class Server;
class Request;
class Response;
class ServerConfig;

struct s_Router_Root {
	bool			set;
	bool			isAlias;
	std::string		path;
	std::string		nearest_root;
};

struct s_Router_Redirection {
	bool			enabled;
	int				status;
	std::string		path;
	std::string		data;
};

struct s_Router_Location {
	std::string		path;
	std::string		modifier;
	bool			strict;

	s_Router_Location() : path("/"), modifier(""),  strict(false) {}
};

struct s_CGI_Data {
	bool			enabled;
	std::string		path;
	t_mapss			params;
};

struct s_Router_Headers {
	bool			enabled;

	struct s_Router_Header {
		std::string		key;
		std::string		value;
		bool			always;
	};

	std::vector<struct s_Router_Header>	list;
};

struct s_Router_Client_Body {
	bool			set;
	size_t			size; // in bytes
};

struct s_Router_Method {
	bool						enabled;
	std::vector<std::string>	methods;
};

typedef struct s_Router_Headers::s_Router_Header	Router_Header_t;

class Router
{
private:
	Router							*_parent;
	struct s_Router_Location		_location;
	struct s_Router_Root			_root;
	struct s_Router_Redirection		_redirection;
	struct s_CGI_Data				_cgi;
	struct s_Router_Headers			_headers;
	struct s_Router_Client_Body		_client_body;
	struct s_Router_Method			_allowed_methods;
	bool							_autoindex;
	std::vector<std::string>		_index;
	std::map<int, std::string>		_error_page;

	std::vector<Router *>			_routes;

	std::string	&checkLeadingTrailingSlash(std::string &str);
	
	const std::string		getDirList(const std::string dirpath, std::string reqPath);

	static std::map<std::string, void (Router::*)(Request &, Response &)>	_method_handlers;
	static std::map<std::string, void (Router::*)(Request &, Response &)>	initMethodHandlers();

	void	reloadChildren(void);

	bool	handleRoutes(Request &request, Response &response);
	void	call(std::string method, Request &request, Response &response);

	void	handleGETMethod(Request &request, Response &response);
	void	handleHEADMethod(Request &request, Response &response);
	void	handlePOSTMethod(Request &request, Response &response);
	void	handlePUTMethod(Request &request, Response &response);
	void	handleDELETEMethod(Request &request, Response &response);
	void	handleTRACEMethod(Request &request, Response &response);

	void	handleCGI(Request &request, Response &response);

	bool	matchRoute(const std::string &route, Response &response) const;

public:
	Router(Router *parent, const struct s_Router_Location location, int level = 0);
	~Router(void);

	const int	level;

	void	reload(void);

	void	route(Request &request, Response &response);

	void					use(Router *router);
	std::vector<Router *>	&getRoutes(void);

	void	allowMethod(const std::string method);
	void	allowMethod(const std::vector<std::string> method);

	void	setRoot(const std::string path);
	void	setAlias(const std::string path);

	bool 			isDefault(void) const;
	Router 			*getParent(void) const;

	const struct s_Router_Location	&getLocation(void) const;

	const std::string				&getRoot(void) const;
	const struct s_Router_Root		&getRootData(void) const;

	std::string			getLocalFilePath(const std::string &requestPath);

	void	setRedirection(const std::string to, int status = 302);
	void	setAutoIndex(const bool autoindex);
	
	const struct s_Router_Redirection	&getRedirection(void) const;

	void	setIndex(const std::vector<std::string> index);
	void	addIndex(const std::string index);

	void	addHeader(const std::string key, const std::string value, const bool always = false);
	const std::vector<Router_Header_t>	&getHeaders(void) const;

	void					setErrorPage(const int code, const std::string path);
	const std::string		&getErrorPage(const int status) const;
	bool					hasErrorPage(const int code) const;

	void 		setClientMaxBodySize(const std::string &size);
	void 		setClientMaxBodySize(const int size);
	size_t 		getClientMaxBodySize(void) const;

	void				setCGI(const std::string path);
	void				enableCGI(void);
	void				addCGIParam(const std::string key, const std::string value);
	const std::string	&getCGIPath() const;

	void	print(std::ostream &os) const;
};

std::ostream	&operator<<(std::ostream &os, const Router &res);

#endif /* ROUTER_HPP */