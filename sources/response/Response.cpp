/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:34 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 13:14:22 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

int		pathIsFile(const std::string& path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0 )
	{
		if (s.st_mode & S_IFDIR)
			return 0;
		else if (s.st_mode & S_IFREG)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

Response::Response(int socket, std::string version): _sent(false), _status(200), _version(version)
{
	this->_socket = socket;
	this->initCodes();
	this->setHeader("Server", "42-webserv");
}

Response::Response(int socket, std::string version, int status = 200): _sent(false), _status(status), _version(version)
{
	this->_socket = socket;
	this->initCodes();
	this->setHeader("Server", "42-webserv");
	if (status != 200)
	{
		this->end();
	}
}

Response::~Response(void)
{
	if (!this->_sent)
		this->end();
}

Response	&Response::status(const uint16_t status)
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

	if (pathIsFile(filepath))
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
	
	res = "HTTP/" + this->_version + " " + std::to_string(this->_status) + " " + this->_res_codes[this->_status] + "\n";
	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it) {
        res += it->first + ": " + it->second + "\n";
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

void	Response::initCodes()
{
	this->_res_codes[100] = "Continue";
	this->_res_codes[101] = "Switching Protocols";
	this->_res_codes[200] = "OK";
	this->_res_codes[201] = "Created";
	this->_res_codes[204] = "No Content";
	this->_res_codes[301] = "Moved Permanently";
	this->_res_codes[302] = "Found";
	this->_res_codes[310] = "Too many Redirects";
	this->_res_codes[400] = "Bad Request";
	this->_res_codes[401] = "Unauthorized";
	this->_res_codes[403] = "Forbidden";
	this->_res_codes[404] = "Not Found";
	this->_res_codes[405] = "Method Not Allowed";
	this->_res_codes[413] = "Payload Too Large";
	this->_res_codes[418] = "I’m a teapot";
	this->_res_codes[500] = "Internal Server Error";
	this->_res_codes[505] = "HTTP Version not supported";
}

const bool	Response::canSend(void)
{
	return (!this->_sent);
}
