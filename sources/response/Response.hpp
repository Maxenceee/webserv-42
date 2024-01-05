/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:17 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 13:12:14 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"

class Response
{
private:
	int										_socket;
	bool									_sent;
	uint16_t								_status;
	std::map<std::string, std::string>		_headers;
	std::string								_body;
	std::string								_version;
	
	std::map<int, std::string>				_res_codes;

	const std::string	prepareResponse(void);
	void				initCodes();

public:
	Response(int socket, std::string version);
	Response(int socket, std::string version, int status);
	~Response(void);

	Response		&status(const uint16_t code);
	Response		&send(const std::string data);
	Response		&sendFile(const std::string filepath);
	Response		&end();

	Response		&setHeader(const std::string header, const std::string value);

	const bool		canSend(void);
};
