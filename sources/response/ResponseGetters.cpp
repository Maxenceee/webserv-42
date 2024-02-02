/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseGetters.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 18:19:44 by mgama             #+#    #+#             */
/*   Updated: 2024/02/02 14:23:00 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

std::ostream	&operator<<(std::ostream &os, const Response &res)
{
	t_mapss::const_iterator	it;

	os << B_GREEN"HTTP" << RESET << "/" << res.getVersion() << " " << res.getSatus() << " " << res.getSatusName() << "\n";

	os << B_CYAN"Headers: " << RESET << std::endl;
	const t_mapss &headers = res.getHeaders();
	for (it = headers.begin(); it != headers.end(); it++)
		os << "\t" << it->first << ": " << it->second << "\n";

	os << B_CYAN"Cookie: " << RESET << std::endl;
	const t_mapss &cookies = res.getCookies();
	for (it = cookies.begin(); it != cookies.end(); it++)
		os << "\t" << it->second << "\n";

	os << B_CYAN"Body:" << RESET << "\n" << res.getBody() << "\n";

	return os;
}

const bool	Response::canSend(void) const
{
	return (!this->_sent);
}

const std::string	Response::getVersion(void) const
{
	return (this->_version);
}

const int			Response::getSatus(void) const
{
	return (this->_status);
}

const std::string	Response::getSatusName(void) const
{
	return (this->_res_codes[this->_status]);
}

const t_mapss	Response::getHeaders(void) const
{
	return (this->_headers);
}

const t_mapss	Response::getCookies(void) const
{
	return (this->_cookie);
}

const std::string	Response::getBody(void) const
{
	return (this->_body);
}

std::string	Response::formatMethods(std::vector<std::string> methods)
{
	return (join(methods, ", "));
}
