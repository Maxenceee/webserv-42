/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:15 by mgama             #+#    #+#             */
/*   Updated: 2024/01/11 19:55:26 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "server/Server.hpp"

#define REQ_SUCCESS		0
#define REQ_ERROR		1

class Server;

class Request
{
private:
	const Server				&_server;
	int							_status;
	const std::string			&_raw;
	const int					_socket;
	std::string					_version;
	std::string					_method;
	std::string					_path;
	std::string					_host;
	int							_port;
	t_mapss						_query;
	t_mapss						_headers;
	t_mapss						_cookie;
	std::string					_body;
	const sockaddr_in			_clientAddr;
	std::string					_ip;

	int		parse(void);
	int		getRequestLine(const std::string &str, size_t &i);
	int		getRequestPath(const std::string &str);
	int		getRequestVersion(const std::string &str);
	int		getRequestHeadersAndBody(const std::string &str, size_t &i);
	int		getRequestQuery(void);
	int		getRequestHostname(const std::string &host);
	int		getRequestCookies(void);

	std::string	nextLine(const std::string &str, size_t& i);

public:
	Request(const Server &server, const std::string &str, int socket, sockaddr_in clientAddr);
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
	const int				getClientSocket(void) const;
	const sockaddr_in		getClientAddr(void) const;
	const std::string		getIP(void) const;
};

std::ostream	&operator<<(std::ostream &os, const Request &req);
