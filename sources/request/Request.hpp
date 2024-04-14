/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:15 by mgama             #+#    #+#             */
/*   Updated: 2024/04/14 19:17:13 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webserv.hpp"
#include "server/Server.hpp"

#define REQ_SUCCESS		0
#define REQ_ERROR		1

class Server;

class Request
{
private:
	// std::string					_raw;
	// const std::string			&_raw;
	bool						_has_request_line;
	// bool						_header_detected;
	bool						_body_detected;
	int							_status;
	const int					_socket;
	std::string					_version;
	std::string					_method;
	std::string					_path;
	std::string					_raw_path;
	std::string					_host;
	int							_port;
	t_mapss						_query;
	t_mapss						_headers;
	t_mapss						_cookie;
	const sockaddr_in			_clientAddr;
	std::string					_body;
	std::string					_ip;

	long long					request_time;

	int		parse(void);
	int		getRequestLine(const std::string &str);
	int		getRequestPath(const std::string &str);
	int		getRequestVersion(const std::string &str);
	int		getRequestHeadersAndBody(const std::string &str);
	int		getRequestQuery(void);
	int		getRequestHostname(const std::string &host);
	int		getRequestCookies(void);

	bool	isValidHeader(const std::string& line);

	std::string	nextLine(const std::string &str, size_t& i);

	void	processChunk(void);

public:
	// Request(const Server &server, const std::string &str, int socket, sockaddr_in clientAddr);
	Request(int socket, sockaddr_in clientAddr);
	~Request(void);

	int						processLine(const std::string &line);

	// void				pushData(char *data, size_t len);
	// void				processRequest(void);

	const std::string		&getMethod(void) const;
	const std::string		&getVersion(void) const;
	const std::string		&getPath(void) const;
	const std::string		&getRawPath(void) const;
	int						getPort(void) const;
	const std::string		&getHost(void) const;
	const t_mapss			&getQueries(void) const;
	const std::string		getQueryString(void) const;
	const t_mapss			&getHeaders(void) const;
	const std::string		&getHeader(const std::string name) const;
	const t_mapss			&getCookies(void) const;
	const std::string		&getBody(void) const;
	int						getStatus(void) const;
	int						getClientSocket(void) const;
	const sockaddr_in		&getClientAddr(void) const;
	const std::string		&getIP(void) const;
	time_t					getRequestTime(void) const;
};

std::ostream	&operator<<(std::ostream &os, const Request &req);

#endif /* REQUEST_HPP */