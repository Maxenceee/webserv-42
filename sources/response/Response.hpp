/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:17 by mgama             #+#    #+#             */
/*   Updated: 2024/09/18 15:14:49 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "webserv.hpp"
#include "server/Server.hpp"
#include "request/Request.hpp"

class Server;
class Request;

struct wbs_cookie_options {
    std::string		path;
    std::string		domain;
    int				maxAge;
    bool			secure;
    bool			httpOnly;

    wbs_cookie_options() : path("/"), domain(""), maxAge(-1), secure(false), httpOnly(false) {}
};

class Response
{
private:
	int					_socket;
	bool				_sent;
	int					_status;
	std::string			_version;
	std::string			_method;
	std::string			_path;
	wbs_mapss_t			_headers;
	wbs_mapss_t			_cookie;
	std::string			_body;
	bool				_upgrade_to_socket;
	
	static wbs_mapis_t	initCodes();
	
	const std::string	prepareResponse(void);
	std::string			getTime(void);

public:
	Response(int socket, const Request &req);
	~Response(void);

	static wbs_mapis_t	http_codes;

	static bool		isValidStatus(int status);

	Response		&status(const int code);
	Response		&send(const std::string data);
	Response		&sendFile(const std::string filepath);
	Response		&sendNotFound(const int code = 404);
	Response		&sendDefault(const int code = -1);
	Response		&redirect(const std::string &path, int status = 302);
	Response		&sendCGI(const std::string data);
	Response		&end(void);
	Response		&upgrade(void);

	Response		&setHeader(const std::string header, const std::string value);
	Response		&setCookie(const std::string name, const std::string value, const wbs_cookie_options &options = wbs_cookie_options());

	Response		&clearBody(void);
	bool			hasBody(void) const;
	bool			isUpgraded(void) const;

	bool			canAddHeader(void) const;

	const std::string		&getVersion(void) const;
	int						getStatus(void) const;
	const std::string		&getSatusName(void) const;
	const wbs_mapss_t		&getHeaders(void) const;
	const wbs_mapss_t		&getCookies(void) const;
	const std::string		&getBody(void) const;

	bool					canSend(void) const;
	void					cancel(void);

	static std::string	formatMethods(const std::vector<std::string> methods);
};

std::ostream	&operator<<(std::ostream &os, const Response &req);

#endif /* RESPONSE_HPP */