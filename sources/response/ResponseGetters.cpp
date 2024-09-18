/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseGetters.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 18:19:44 by mgama             #+#    #+#             */
/*   Updated: 2024/09/18 12:43:43 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

std::ostream	&operator<<(std::ostream &os, const Response &res)
{
	wbs_mapss_t::const_iterator	it;

	os << B_GREEN"HTTP" << RESET << "/" << res.getVersion() << " ";
	switch (res.getStatus() / 100)
	{
	case 1:
		os << GREY;
		break;
	case 2:
		os << GREEN;
		break;
	case 3:
		os << CYAN;
		break;
	case 4:
		os << YELLOW;
		break;
	case 5:
	default:
		os << RED;
		break;
	}
	os << res.getStatus() << RESET << " " << res.getSatusName() << "\n";

	os << B_CYAN"Headers: " << RESET << std::endl;
	const wbs_mapss_t &headers = res.getHeaders();
	for (it = headers.begin(); it != headers.end(); it++)
		os << "\t" << it->first << ": " << it->second << "\n";

	os << B_CYAN"Cookie: " << RESET << std::endl;
	const wbs_mapss_t &cookies = res.getCookies();
	for (it = cookies.begin(); it != cookies.end(); it++)
		os << "\t" << it->second << "\n";

	os << B_CYAN"Body:" << RESET << "\n" << cropoutputs(res.getBody()) << "\n";

	return os;
}

bool	Response::canSend(void) const
{
	return (!this->_sent);
}

const std::string	&Response::getVersion(void) const
{
	return (this->_version);
}

int		Response::getStatus(void) const
{
	return (this->_status);
}

const std::string	&Response::getSatusName(void) const
{
	return (this->http_codes[this->_status]);
}

const wbs_mapss_t	&Response::getHeaders(void) const
{
	return (this->_headers);
}

const wbs_mapss_t	&Response::getCookies(void) const
{
	return (this->_cookie);
}

const std::string	&Response::getBody(void) const
{
	return (this->_body);
}

std::string	Response::formatMethods(std::vector<std::string> methods)
{
	return (join(methods, ", "));
}
