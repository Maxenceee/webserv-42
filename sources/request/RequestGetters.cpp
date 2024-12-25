/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestGetters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 12:36:57 by mgama             #+#    #+#             */
/*   Updated: 2024/12/25 18:41:50 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

std::ostream	&operator<<(std::ostream &os, const Request &req)
{
	wbs_mapss_t::const_iterator	it;

	os << B_CYAN"Method: " << RESET << req.getMethod() << " | HTTP version: " << req.getVersion() << "\n";
	os << B_CYAN"Path: " << RESET << req.getRawPath() << "\n";
	os << B_CYAN"Host: " << RESET << req.getHost() << "\n";
	os << B_CYAN"Port: " << RESET << req.getPort() << "\n";
	os << B_CYAN"Origin IP: " << RESET << req.getIP() << "\n";

	os << B_CYAN"Headers: " << RESET << std::endl;
	const wbs_mapss_t &headers = req.getHeaders();
	for (it = headers.begin(); it != headers.end(); it++)
		os << "\t" << it->first << ": " << it->second << "\n";

	os << B_CYAN"Query: " << RESET << std::endl;
	const wbs_mapss_t &queries = req.getQueries();
	for (it = queries.begin(); it != queries.end(); it++)
		os << "\t" << it->first << ": " << it->second << "\n";

	os << B_CYAN"Cookie: " << RESET << std::endl;
	const wbs_mapss_t &cookies = req.getCookies();
	for (it = cookies.begin(); it != cookies.end(); it++)
		os << "\t" << it->first << ": " << it->second << "\n";

	os << B_CYAN"Body:"  << RESET << "\n" << cropoutput(req.getBody()) << "\n";

	return (os);
}

const std::string	&Request::getMethod(void) const
{
	return (this->_method);
}

const std::string	&Request::getVersion(void) const
{
	return (this->_version);
}

const std::string	&Request::getPath(void) const
{
	return (this->_path);
}

const std::string	&Request::getRawPath(void) const
{
	return (this->_raw_path);
}

const std::string	&Request::getHost(void) const
{
	return (this->_host);
}

int			Request::getPort(void) const
{
	return (this->_port);
}

const wbs_mapss_t	&Request::getQueries(void) const
{
	return (this->_query);
}

const std::string	Request::getQueryString(void) const
{
	std::string		query;
	for (wbs_mapss_t::const_iterator it = this->_query.begin(); it != this->_query.end(); it++)
	{
		query += it->first + "=" + it->second;
		if (it != --this->_query.end())
			query += "&";
	}
	return (query);
}

const wbs_mapss_t	&Request::getHeaders(void) const
{
	return (this->_headers);
}

bool	Request::hasHeader(const std::string &name) const
{
	return (this->_headers.count(name) != 0);
}

const std::string	&Request::getHeader(const std::string &name) const
{
	return (this->_headers.at(name));
}

const wbs_mapss_t	&Request::getCookies(void) const
{
	return (this->_cookie);
}

const std::string	&Request::getBody(void) const
{
	return (this->_body);
}

int	Request::getStatus(void) const
{
	return (this->_status);
}

int	Request::getClientSocket(void) const
{
	return (this->_socket);
}

const sockaddr_in	&Request::getClientAddr(void) const
{
	return (this->_clientAddr);
}

const std::string	&Request::getIP(void) const
{
	return (this->_ip);
}

const std::string	&Request::getRawRequest(void) const
{
	return (this->_raw);
}

const struct wbs_request_time	&Request::getRequestTime(void) const
{
	return (this->request_time);
}

const std::string	Request::prepareForProxying(void) const
{
	std::string		res;

	res = this->_method + " " + this->_raw_path + " " + "HTTP/" + this->_version + WBS_CRLF;
	for (wbs_mapss_t::const_iterator it = this->_headers.begin(); it != this->_headers.end(); it++)
		res += it->first + ": " + it->second + WBS_CRLF;
	res += WBS_CRLF;
	res += this->_body;
	return (res);
}
