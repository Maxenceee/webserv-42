/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:52:36 by mgama             #+#    #+#             */
/*   Updated: 2024/12/16 16:17:10 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "webserv.hpp"
#include "Server.hpp"
#include "logger/Logger.hpp"

class Server;
class Request;
class Response;
class Router;

struct wbs_server_name {
	std::string		name;
	int				port;	
};

struct wbs_server_ssl {
	bool			enabled;
	std::string		cert_file;
	std::string		key_file;
	std::string		ciphers;
	SSL_CTX			*ctx;
};

typedef std::vector<struct wbs_server_name>	wbs_server_names;

class ServerConfig
{
private:
	Server					*_server;
	uint32_t				_address;
	uint16_t				_port;
	struct wbs_server_ssl	_ssl;
	wbs_server_names		_server_name;

	// les comportements par default du serveur sont stockés dans un router spécifique
	Router				*_default;

public:
	ServerConfig(Server *server = NULL);
	~ServerConfig(void);

	bool		used;

	void		setServer(Server *server);

	// void		handleRoutes(Request &req, Response &res);

	Router		*getDefaultHandler(void);
	void		use(Router *router);

	void		setAddress(const std::string &address);
	void		setAddress(const uint32_t address);
	uint32_t	getAddress(void) const;

	void		setPort(const uint16_t port);
	uint16_t	getPort(void) const;

	void		setSSL(bool ssl);
	bool		hasSSL(void) const;
	bool		setupSSL(void);
	void		setSSLCertFile(const std::string &cert_file);
	void		setSSLKeyFile(const std::string &key_file);
	void		setSSLCiphers(const std::string &ciphers);
	SSL_CTX		*getSSLCTX(void) const;

	
	void					addNames(const std::vector<std::string> &name);
	void					addName(const std::string &name);
	const wbs_server_names	&getNames(void) const;
	bool					evalName(const std::string &name) const;

	void	print(std::ostream &os) const;
};

std::ostream	&operator<<(std::ostream &os, const ServerConfig &config);

#endif /* SERVERCONFIG_HPP */