/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:17 by mgama             #+#    #+#             */
/*   Updated: 2024/02/02 22:11:59 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webserv.hpp"
#include "server/Server.hpp"
#include "request/Request.hpp"

class Server;
class Request;

struct CookieOptions {
    std::string		path;
    std::string		domain;
    int				maxAge;
    bool			secure;
    bool			httpOnly;

    CookieOptions() : path("/"), domain(""), maxAge(-1), secure(false), httpOnly(false) {}
};

class Response
{
private:
	const Server		&_server;
	int					_socket;
	bool				_sent;
	int					_status;
	std::string			_version;
	std::string			_method;
	std::string			_path;
	t_mapss				_headers;
	t_mapss				_cookie;
	std::string			_body;

	// t_mapss				_static_dir;
	
	static std::map<int, std::string>		_res_codes;
	static std::map<int, std::string>		initCodes();
	
	const std::string	prepareResponse(void);
	std::string			getTime(void);

public:
	// Response(const Server &server, int socket, t_mapss static_dir, std::string version, int status = 200);
	Response(const Server &server, int socket, const Request &req);
	~Response(void);

	Response		&status(const int code);
	Response		&send(const std::string data);
	Response		&sendFile(const std::string filepath);
	Response		&sendNotFound(const int code = 404);
	// Response		&render(const std::string filename);
	Response		&redirect(const std::string &path, int status = 302);
	Response		&end();

	Response		&setHeader(const std::string header, const std::string value);
	Response		&setCookie(const std::string name, const std::string value, const CookieOptions &options = CookieOptions());

	Response		&clearBody(void);

	const std::string		getVersion(void) const;
	const int				getSatus(void) const;
	const std::string		getSatusName(void) const;
	const t_mapss			getHeaders(void) const;
	const t_mapss			getCookies(void) const;
	const std::string		getBody(void) const;

	const bool		canSend(void) const;

	static std::string	formatMethods(const std::vector<std::string> methods);
};

std::ostream	&operator<<(std::ostream &os, const Response &req);

#endif /* RESPONSE_HPP */