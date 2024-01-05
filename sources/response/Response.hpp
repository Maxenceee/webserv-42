/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:17 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 18:45:03 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"

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
	int										_socket;
	bool									_sent;
	int										_status;
	std::string								_version;
	std::map<std::string, std::string>		_headers;
	std::map<std::string, std::string>		_cookie;
	std::string								_body;
	
	static std::map<int, std::string>		_res_codes;
	static std::map<int, std::string>		initCodes();

public:
	const std::string	prepareResponse(void);
	Response(int socket, std::string version);
	Response(int socket, std::string version, int status);
	~Response(void);

	Response		&status(const int code);
	Response		&send(const std::string data);
	Response		&sendFile(const std::string filepath);
	Response		&end();

	Response		&setHeader(const std::string header, const std::string value);
	Response		&setCookie(const std::string name, const std::string value, const CookieOptions &options = CookieOptions());

	const std::string							getVersion(void) const;
	const int									getSatus(void) const;
	const std::string							getSatusName(void) const;
	const std::map<std::string, std::string>	getHeaders(void) const;
	const std::map<std::string, std::string>	getCookies(void) const;
	const std::string							getBody(void) const;

	const bool		canSend(void) const;
};

std::ostream	&operator<<(std::ostream &os, const Response &req);