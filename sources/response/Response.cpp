/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:34 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 21:48:08 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

std::map<int, std::string>	Response::_res_codes = Response::initCodes();

Response::Response(int socket, t_mapss static_dir, std::string version, int status): _sent(false), _static_dir(static_dir), _status(status), _version(version)
{
	this->_socket = socket;
	this->initCodes();
	this->setHeader("Server", "42-webserv");
	if (status != 200)
	{
		this->end();
	}
}

std::map<int, std::string>	Response::initCodes()
{
	std::map<int, std::string>	codes;
	
	codes[100] = "Continue";
	codes[101] = "Switching Protocols";
	codes[200] = "OK";
	codes[201] = "Created";
	codes[204] = "No Content";
	codes[301] = "Moved Permanently";
	codes[302] = "Found";
	codes[310] = "Too many Redirects";
	codes[400] = "Bad Request";
	codes[401] = "Unauthorized";
	codes[403] = "Forbidden";
	codes[404] = "Not Found";
	codes[405] = "Method Not Allowed";
	codes[413] = "Payload Too Large";
	codes[418] = "I’m a teapot";
	codes[500] = "Internal Server Error";
	codes[505] = "HTTP Version not supported";
	return (codes);
}

Response::~Response(void)
{
	if (!this->_sent)
		this->end();
}

Response	&Response::status(const int status)
{
	this->_status = status;
	return (*this);
}

Response	&Response::send(const std::string data)
{
	this->_body = data;
	this->setHeader("Content-Length", std::to_string(this->_body.size()));
	return (*this);
}

Response	&Response::sendFile(const std::string filepath)
{
	std::ofstream		file;
	std::stringstream	buffer;

	if (isFile(filepath))
	{
		file.open(filepath.c_str(), std::ifstream::in);
		if (file.is_open() == false)
		{
			this->status(500);
			this->setHeader("Content-Type", "text/html");
			this->_body = "<!DOCTYPE html>\n<html><title>Error</title><body>There was an error finding your file</body></html>";
			return (*this);
		}
		buffer << file.rdbuf();
		file.close();
		this->setHeader("Content-Type", "text/html");
		this->_body = buffer.str();
	}
	else
	{
		this->status(500);
		this->setHeader("Content-Type", "text/html");
		this->_body = "<!DOCTYPE html>\n<html><title>Error</title><body>There was an error finding your file</body></html>";
	}
	return (*this);
}

/**
 * A voir si on la garde
 */
// Response	&Response::render(const std::string filename)
// {
// 	if (!this->_static_dir.count(filename))
// 	{
// 		this->status(500);
// 		this->setHeader("Content-Type", "text/html");
// 		this->_body = "<!DOCTYPE html>\n<html><title>Error</title><body>There was an error finding your file</body></html>";
// 	}
// 	this->sendFile(this->_static_dir[filename]);
// 	return (*this);
// }

Response	&Response::end()
{
	if (!this->_sent)
	{
		std::string	res = this->prepareResponse();
		int ret = ::send(this->_socket, res.c_str(), res.size(), 0);
		this->_sent = true;
	}
	else
	{
		std::cerr << "Response error: cannot set header it was sent" << std::endl;
	}
	return (*this);
}

const std::string	Response::prepareResponse(void)
{
	std::string	res;
	
	res = "HTTP/" + this->_version + " " + std::to_string(this->_status) + " " + this->getSatusName() + "\n";
	for (t_mapss::iterator it = this->_headers.begin(); it != this->_headers.end(); it++) {
		res += it->first + ": " + it->second + "\n";
	}
	for (t_mapss::iterator it = this->_cookie.begin(); it != this->_cookie.end(); it++) {
		res += "Set-Cookie: " + it->second + "\n";
	}
	res += "\n";
	res += this->_body + "\n";
	return (res);
}

Response	&Response::setHeader(const std::string header, const std::string value)
{
	this->_headers[header] = value;
	return (*this);
}

Response	&Response::setCookie(const std::string name, const std::string value, const CookieOptions &options)
{
	std::string cookieStr = name + "=" + value;

	cookieStr += "; path=" + (!options.path.empty() ? options.path : "/");

	if (!options.domain.empty()) {
		cookieStr += "; domain=" + options.domain;
	}

	if (options.maxAge >= 0) {
		cookieStr += "; Max-Age=" + std::to_string(options.maxAge);
	}

	if (options.secure) {
		cookieStr += "; secure";
	}

	if (options.httpOnly) {
		cookieStr += "; HttpOnly";
	}

	this->_cookie[name] = cookieStr;
	return (*this);
}
