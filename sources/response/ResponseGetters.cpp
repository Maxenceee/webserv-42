/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseGetters.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 18:19:44 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 18:48:08 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

std::ostream	&operator<<(std::ostream &os, const Response &res)
{
	std::map<std::string, std::string>::const_iterator	it;

	os << B_GREEN"HTTP" << RESET << "/" << res.getVersion() << " " << res.getSatus() << " " << res.getSatusName() << "\n";

	os << B_CYAN"Headers: " << RESET << std::endl;
	const std::map<std::string, std::string> &headers = res.getHeaders();
	for (it = headers.begin(); it != headers.end(); it++)
		os << "\t" << it->first << ": " << it->second << "\n";

	os << B_CYAN"Cookie: " << RESET << std::endl;
	const std::map<std::string, std::string> &cookies = res.getCookies();
	for (it = cookies.begin(); it != cookies.end(); it++)
		os << "\t" << it->second << "\n";

	os << B_CYAN"Body:" << RESET << "\n" << res.getBody() << "\n";

	return os;
}

const bool	Response::canSend(void) const
{
	return (!this->_sent);
}

const std::string		Response::getVersion(void) const
{
	return (this->_version);
}

const int				Response::getSatus(void) const
{
	return (this->_status);
}

const std::string		Response::getSatusName(void) const
{
	return (this->_res_codes[this->_status]);
}

const std::map<std::string, std::string>	Response::getHeaders(void) const
{
	return (this->_headers);
}

const std::map<std::string, std::string>	Response::getCookies(void) const
{
	return (this->_cookie);
}

const std::string		Response::getBody(void) const
{
	return (this->_body);
}
