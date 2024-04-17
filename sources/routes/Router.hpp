/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 12:04:59 by mgama             #+#    #+#             */
/*   Updated: 2024/04/17 21:21:21 by mgama            ###   ########.fr       */
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

struct wbs_router_root {
	bool			set;
	bool			isAlias;
	std::string		path;
	std::string		nearest_root;
};

struct wbs_router_redirection {
	bool			enabled;
	int				status;
	std::string		path;
	std::string		data;
};

struct wbs_router_location {
	std::string		path;
	std::string		modifier;
	bool			strict;

	wbs_router_location() : path("/"), modifier(""),  strict(false) {}
};

struct wbs_router_cgi_data {
	bool			enabled;
	std::string		path;
	t_mapss			params;
};

struct wbs_router_headers {
	bool			enabled;

	struct wbs_router_header {
		std::string		key;
		std::string		value;
		bool			always;
	};

	std::vector<struct wbs_router_header>	list;
};

struct wbs_router_client_body {
	bool			set;
	size_t			size; // in bytes
};

struct wbs_router_method {
	bool						enabled;
	std::vector<std::string>	methods;
};

struct wbs_router_proxy {
	bool			enabled;
	std::string		host;
	int				port;
	bool			buffering;

	std::map<std::string, std::string>	headers;

	wbs_router_proxy() : enabled(false), host(""), port(0), buffering(false) {}
};

typedef struct wbs_router_headers::wbs_router_header	wbs_router_header_t;

class Router
{
private:
	Router							*_parent;
	struct wbs_router_location		_location;
	struct wbs_router_root			_root;
	struct wbs_router_redirection	_redirection;
	struct wbs_router_cgi_data		_cgi;
	struct wbs_router_headers		_headers;
	struct wbs_router_client_body	_client_body;
	struct wbs_router_method		_allowed_methods;
	bool							_autoindex;
	std::vector<std::string>		_index;
	std::map<int, std::string>		_error_page;
	struct wbs_router_proxy			_proxy;

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

	void	handleProxy(Request &request, Response &response);

	bool	matchRoute(const std::string &route, Response &response) const;

public:
	Router(Router *parent, const struct wbs_router_location location, int level = 0);
	~Router(void);

	const int	level;

	void	reload(void);

	Router	*eval(const std::string &path, const std::string &method, Response &response);
	void	route(Request &request, Response &response);

	void					use(Router *router);
	std::vector<Router *>	&getRoutes(void);

	void	allowMethod(const std::string method);
	void	allowMethod(const std::vector<std::string> method);

	void	setRoot(const std::string path);
	void	setAlias(const std::string path);

	bool 			isDefault(void) const;
	Router 			*getParent(void) const;

	const struct wbs_router_location	&getLocation(void) const;

	const std::string					&getRoot(void) const;
	const struct wbs_router_root		&getRootData(void) const;

	std::string			getLocalFilePath(const std::string &requestPath);

	void	setRedirection(const std::string to, int status = 302);
	void	setAutoIndex(const bool autoindex);
	
	const struct wbs_router_redirection	&getRedirection(void) const;

	void	setIndex(const std::vector<std::string> index);
	void	addIndex(const std::string index);

	void	addHeader(const std::string key, const std::string value, const bool always = false);
	const std::vector<wbs_router_header_t>	&getHeaders(void) const;

	void					setErrorPage(const int code, const std::string path);
	const std::string		&getErrorPage(const int status) const;
	bool					hasErrorPage(const int code) const;

	void 		setClientMaxBodySize(const std::string &size);
	void 		setClientMaxBodySize(const int size);
	size_t 		getClientMaxBodySize(void) const;
	bool		hasClientMaxBodySize(void) const;

	void				setCGI(const std::string path);
	void				enableCGI(void);
	void				addCGIParam(const std::string key, const std::string value);
	const std::string	&getCGIPath() const;

	void				sendResponse(Response &response);

	bool				isProxy(void) const;

	void	print(std::ostream &os) const;
};

std::ostream	&operator<<(std::ostream &os, const Router &res);

#endif /* ROUTER_HPP */