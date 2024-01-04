/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:15 by mgama             #+#    #+#             */
/*   Updated: 2024/01/04 21:37:38 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"

class Request
{
private:
	std::string								_method;
	std::string								_host;
	std::string								_version;
	std::map<std::string, std::string>		_headers;
	std::string								_body;
	std::string								_path;
	std::string								_query;
	const std::string						&_raw;

public:
	Request(const std::string &str);
	~Request(void);
};
