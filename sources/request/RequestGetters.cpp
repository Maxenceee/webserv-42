/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestGetters.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 12:36:57 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 14:20:45 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

std::ostream	&operator<<(std::ostream &os, const Request &req)
{
	std::map<std::string, std::string>::const_iterator	it;

	os << "Method: " << req.getMethod() << " | HTTP version: " << req.getVersion() << '\n';
	os << "Path: " << req.getPath() << '\n';
	os << "Host: " << req.getHost() << '\n';
	os << "Port: " << req.getPort() << '\n';

	os << "Headers: \n";
	const std::map<std::string, std::string> &headers = req.getHeaders();
	for (it = headers.begin(); it != headers.end(); it++)
		os << "\t" << it->first << ": " << it->second << '\n';

	os << "Query: \n";
	const std::map<std::string, std::string> &queries = req.getQueries();
	for (it = queries.begin(); it != queries.end(); it++)
		os << "\t" << it->first << ": " << it->second << '\n';

	// os << '\n' << "Body:\n" << req.getBody() << '\n';

	return os;
}

const std::string		Request::getMethod(void) const
{
	return (this->_method);
}

const std::string		Request::getVersion(void) const
{
	return (this->_version);
}

const std::string		Request::getPath(void) const
{
	return (this->_path);
}

const std::string		Request::getHost(void) const
{
	return (this->_host);
}

const std::string		Request::getPort(void) const
{
	return (this->_port);
}

const std::map<std::string, std::string>	Request::getQueries(void) const
{
	return (this->_query);
}

const std::map<std::string, std::string>	Request::getHeaders(void) const
{
	return (this->_headers);
}

const std::string		Request::getBody(void) const
{
	return (this->_body);
}

const int		Request::getSatus(void) const
{
	return (this->_status);
}
