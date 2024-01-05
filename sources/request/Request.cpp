/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:33 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 14:14:47 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

std::vector<std::string>	Request::methods = Request::initMethods();

Request::Request(const std::string &str): _raw(str), _status(200), _host(""), _body("")
{
	this->resetHeaders();
	this->parse();
}

Request::~Request(void)
{
}

std::vector<std::string>		Request::initMethods()
{
	std::vector<std::string>	methods;

	methods.push_back("GET");
	methods.push_back("HEAD");
	methods.push_back("POST");
	methods.push_back("PUT");
	methods.push_back("DELETE");
	methods.push_back("OPTIONS");
	methods.push_back("TRACE" );

	return methods;
}

void	Request::resetHeaders(void)
{
	this->_headers.clear();

// 	this->_headers["Accept-Charsets"] = "";
// 	this->_headers["Accept-Language"] = "";
// 	this->_headers["Allow"] = "";
// 	this->_headers["Auth-Scheme"] = "";
// 	this->_headers["Authorization"] = "";
// 	this->_headers["Content-Language"] = "";
// 	this->_headers["Content-Length"] = "";
// 	this->_headers["Content-Location"] = "";
// 	this->_headers["Content-Type"] = "";
// 	this->_headers["Date"] = "";
// 	this->_headers["Host"] = "";
// 	this->_headers["Last-Modified"] = "";
// 	this->_headers["Location"] = "";
// 	this->_headers["Referer"] = "";
// 	this->_headers["Retry-After"] = "";
// 	this->_headers["Server"] = "";
// 	this->_headers["Transfer-Encoding"] = "";
// 	this->_headers["User-Agent"] = "";
// 	this->_headers["Www-Authenticate"] = "";
// 	this->_headers["Connection"] = "Keep-Alive";
}

int	Request::parse(void)
{
	size_t	i;

	if ((this->_status = this->getRequestLine(this->_raw, i)) != 200)
	{
		return (REQ_ERROR);
	}
	this->getRequestHeadersAndBody(this->_raw, i);
	this->getRequestQuery();
	return (REQ_SUCCESS);
}

int	Request::getRequestLine(const std::string &str, size_t &i)
{
	size_t		j;
	std::string	req_line;

	i = str.find_first_of('\n');
	req_line = str.substr(0, i);
	i += 1;
	j = req_line.find_first_of(' ');

	if (j == std::string::npos)
	{
		std::cerr << B_RED << "RFC error: no space after method" << RESET << std::endl;
		return (400);
	}
	this->_method.assign(req_line, 0, j);
	return (this->getRequestPath(req_line));
}

int	Request::getRequestPath(const std::string &str)
{
	std::vector<std::string>	req_tokens;

	req_tokens = split(str, ' ');
	if (req_tokens.size() < 3)
	{
		std::cerr << B_RED << "RFC error: missing PATH or HTTP version" << RESET << std::endl;
		return (400);
	}
	this->_path = req_tokens[1];
	return (this->getRequestVersion(req_tokens[2]));
}

int	Request::getRequestVersion(const std::string &str)
{
	std::string	http("HTTP/");

	if (str.compare(0, 5, http) == 0)
	{
		this->_version.assign(str, 5, 3);
		if (this->_version != "1.0" && this->_version != "1.1")
		{
			std::cerr << B_RED << "request error: unsupported HTTP version" << RESET << std::endl;
			return (505);
		}
	}
	else
	{
		std::cerr << B_RED << "RFC error: invalid HTTP version" << RESET << std::endl;
		return (400);
	}
	if (!contains(this->methods, this->_method))
		return (405);
	return (200);
}

std::string			Request::nextLine(const std::string &str, size_t &i)
{
	std::string		ret;
	size_t			j;

	if (i == std::string::npos)
		return ("");
	j = str.find_first_of('\n', i);
	ret = str.substr(i, j - i);
	if (ret[ret.size() - 1] == '\r')
		pop(ret);
	i = (j == std::string::npos ? j : j + 1);
	return (ret);
}

int	Request::getRequestHeadersAndBody(const std::string &str, size_t &i)
{
	std::string	key;
	std::string	value;
	std::string	line;

	while ((line = nextLine(str, i)) != "\r" && line != "")
	{
		key = readKey(line);
		value = readValue(line);
		this->_headers[key] = value;
	}
	this->_host = this->_headers["Host"]; // must remove port from raw host before 
	this->_body = str.substr(i, std::string::npos);
	return (REQ_SUCCESS);
}

int	Request::getRequestQuery(void)
{
	size_t		i;
	std::string	query = "";

	i = this->_path.find_first_of('?');
	if (i != std::string::npos)
	{
		query.assign(this->_path, i + 1, std::string::npos);
		this->_path = this->_path.substr(0, i);
	}
	if (!query.size())
		return (REQ_SUCCESS);
	size_t pos = 0;
    while (pos < query.length()) {
        size_t ampersandPos = query.find('&', pos);

        std::string pair;
        if (ampersandPos != std::string::npos) {
            pair = query.substr(pos, ampersandPos - pos);
        } else {
            pair = query.substr(pos);
        }

        size_t equalPos = pair.find('=');

        std::string key = pair.substr(0, equalPos);
        std::string value = "";
        if (equalPos != std::string::npos) {
			value = pair.substr(equalPos + 1);
        }
    	this->_query[key] = value;

        if (ampersandPos != std::string::npos) {
            pos = ampersandPos + 1;
        } else {
            break;
        }
    }
	return (REQ_SUCCESS);
}
