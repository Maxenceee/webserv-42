/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:15 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 19:58:31 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"

#define REQ_SUCCESS		0
#define REQ_ERROR		1

class Request
{
private:
	int							_status;
	const std::string			&_raw;
	std::string					_version;
	std::string					_method;
	std::string					_path;
	std::string					_host;
	int							_port;
	t_mapss						_query;
	t_mapss						_headers;
	t_mapss						_cookie;
	std::string					_body;

	int		parse(void);
	int		getRequestLine(const std::string &str, size_t &i);
	int		getRequestPath(const std::string &str);
	int		getRequestVersion(const std::string &str);
	int		getRequestHeadersAndBody(const std::string &str, size_t &i);
	int		getRequestQuery(void);
	int		getRequestHostname(const std::string &host);
	int		getRequestCookies(void);

	std::string	nextLine(const std::string &str, size_t& i);

	static std::vector<std::string>		methods;
	static std::vector<std::string>		initMethods();

public:
	Request(const std::string &str);
	~Request(void);

	const std::string		getMethod(void) const;
	const std::string		getVersion(void) const;
	const std::string		getPath(void) const;
	const int				getPort(void) const;
	const std::string		getHost(void) const;
	const t_mapss			getQueries(void) const;
	const t_mapss			getHeaders(void) const;
	const t_mapss			getCookies(void) const;
	const std::string		getBody(void) const;
	const int				getSatus(void) const;
};

std::ostream	&operator<<(std::ostream &os, const Request &req);
