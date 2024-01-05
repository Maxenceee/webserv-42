/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:15 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 14:02:23 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"

#define REQ_SUCCESS		0
#define REQ_ERROR		1

class Request
{
private:
	int										_status;
	const std::string						&_raw;
	std::string								_version;
	std::string								_method;
	std::string								_path;
	std::string								_host;
	std::map<std::string, std::string>		_query;
	std::map<std::string, std::string>		_headers;
	std::string								_body;

	void	resetHeaders(void);
	int		parse(void);
	int		getRequestLine(const std::string &str, size_t &i);
	int		getRequestPath(const std::string &str);
	int		getRequestVersion(const std::string &str);
	int		getRequestHeadersAndBody(const std::string &str, size_t &i);
	int		getRequestQuery(void);

	std::string	nextLine(const std::string &str, size_t& i);

	static std::vector<std::string>		methods;
	static std::vector<std::string>		initMethods();

public:
	Request(const std::string &str);
	~Request(void);

	const std::string							getMethod(void) const;
	const std::string							getVersion(void) const;
	const std::string							getPath(void) const;
	const std::string							getHost(void) const;
	const std::map<std::string, std::string>	getQueries(void) const;
	const std::map<std::string, std::string>	getHeaders(void) const;
	const std::string							getBody(void) const;
	const int									getSatus(void) const;
};

std::ostream	&operator<<(std::ostream &os, const Request &req);
