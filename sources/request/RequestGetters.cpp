/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestGetters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 12:36:57 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 20:03:49 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

std::ostream	&operator<<(std::ostream &os, const Request &req)
{
	t_mapss::const_iterator	it;

	os << B_CYAN"Method: " << RESET << req.getMethod() << " | HTTP version: " << req.getVersion() << "\n";
	os << B_CYAN"Path: " << RESET << req.getPath() << "\n";
	os << B_CYAN"Host: " << RESET << req.getHost() << "\n";
	os << B_CYAN"Port: " << RESET << req.getPort() << "\n";

	os << B_CYAN"Headers: " << RESET << std::endl;
	const t_mapss &headers = req.getHeaders();
	for (it = headers.begin(); it != headers.end(); it++)
		os << "\t" << it->first << ": " << it->second << "\n";

	os << B_CYAN"Query: " << RESET << std::endl;
	const t_mapss &queries = req.getQueries();
	for (it = queries.begin(); it != queries.end(); it++)
		os << "\t" << it->first << ": " << it->second << "\n";

	os << B_CYAN"Cookie: " << RESET << std::endl;
	const t_mapss &cookies = req.getCookies();
	for (it = cookies.begin(); it != cookies.end(); it++)
		os << "\t" << it->first << ": " << it->second << "\n";

	os << B_CYAN"Body:"  << RESET << "\n" << req.getBody() << "\n";

	return (os);
}

const std::string	Request::getMethod(void) const
{
	return (this->_method);
}

const std::string	Request::getVersion(void) const
{
	return (this->_version);
}

const std::string	Request::getPath(void) const
{
	return (this->_path);
}

const std::string	Request::getHost(void) const
{
	return (this->_host);
}

const int		Request::getPort(void) const
{
	return (this->_port);
}

const t_mapss	Request::getQueries(void) const
{
	return (this->_query);
}

const t_mapss	Request::getHeaders(void) const
{
	return (this->_headers);
}

const t_mapss	Request::getCookies(void) const
{
	return (this->_cookie);
}

const std::string	Request::getBody(void) const
{
	return (this->_body);
}

const int		Request::getSatus(void) const
{
	return (this->_status);
}
