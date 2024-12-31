/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/26 13:53:09 by mgama             #+#    #+#             */
/*   Updated: 2024/12/31 19:52:55 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

std::ostream	&operator<<(std::ostream &os, const ServerConfig &config)
{
	config.print(os);
	os << std::endl;
	return os;
}

ServerConfig::ServerConfig(Server *server): _server(server), used(false)
{
	this->_default = new Router(NULL, wbs_router_location());
	/**
	 * On verifie si le serveur est lancé en tant que root ou non. Si c'est le cas, on
	 * configure le serveur pour que le port par defaut soit 80, sinon on utilise le port 8080.
	 */
	if (getuid() == 0)
		this->_port = 80;
	else
		this->_port = 8000;
	this->_address = INADDR_ANY;

	/**
	 * Inialisation de la configuration SSL par défaut.
	 */
	this->_ssl.enabled = false;
	this->_ssl.ctx = NULL;
	this->_ssl.ciphers = "HIGH:!aNULL:!MD5";
}

ServerConfig::~ServerConfig(void)
{
	delete this->_default;
	if (this->_ssl.ctx)
		SSL_CTX_free(this->_ssl.ctx);
	Logger::debug("ServerConfig destroyed");
}

void	ServerConfig::setServer(Server *server)
{
	this->_server = server;
}

Router	*ServerConfig::getDefaultHandler(void)
{
	return (this->_default);
}

void	ServerConfig::use(Router *router)
{
	this->_default->use(router);
}

void	ServerConfig::setAddress(const std::string &address)
{
	if (!this->_server || !this->_server->isInit())
	{
		if (address == "*")
			this->_address = INADDR_ANY;
		else
			this->_address = setIPAddress(address);
	}
	else
		Logger::error("server error: could not set address after server startup");
}

void	ServerConfig::setAddress(const uint32_t address)
{
	if (!this->_server || !this->_server->isInit())
		this->_address = address;
	else
		Logger::error("server error: could not set address after server startup");
}

uint32_t	ServerConfig::getAddress(void) const
{
	return (this->_address);
}

void	ServerConfig::setPort(const uint16_t port)
{
	if (!this->_server || !this->_server->isInit())
		this->_port = port;
	else
		Logger::error("server error: could not set port after server startup");
}

uint16_t	ServerConfig::getPort(void) const
{
	return (this->_port);
}

void	ServerConfig::setSSL(bool ssl)
{
	this->_ssl.enabled = ssl;
}

bool	ServerConfig::shouldUseSSL(void) const
{
	return (this->_ssl.enabled);
}

void	ServerConfig::setSSLCertFile(const std::string &cert_file)
{
	this->_ssl.cert_file = cert_file;
}

void	ServerConfig::setSSLKeyFile(const std::string &key_file)
{
	this->_ssl.key_file = key_file;
}

void	ServerConfig::setSSLCiphers(const std::string &ciphers)
{
	this->_ssl.ciphers = ciphers;
}

bool	ServerConfig::setupSSL(void)
{
	if (this->_ssl.ctx != NULL)
	{
		Logger::warning("server warning: SSL context already set");
		return (false);
	}
	if (this->_ssl.cert_file.empty())
	{
		Logger::error("server error: SSL certificate file not set");
		return (false);
	}
	if (this->_ssl.key_file.empty())
	{
		Logger::error("server error: SSL key file not set");
		return (false);
	}
	/**
	 * Création du contexte SSL pour le serveur en utilisant le protocole TLS
	 * (Transport Layer Security).
	 */
	this->_ssl.ctx = SSL_CTX_new(TLS_server_method());
	if (!this->_ssl.ctx)
	{
		Logger::error("server error: could not create SSL context");
		ERR_print_errors_fp(stderr);
		return (false);
	}

	/**
	 * Ajout des fichiers de certificat et de clé privée au contexte SSL.
	 */
	if (SSL_CTX_use_certificate_file(this->_ssl.ctx, this->_ssl.cert_file.c_str(), SSL_FILETYPE_PEM) <= 0 ||
		SSL_CTX_use_PrivateKey_file(this->_ssl.ctx, this->_ssl.key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
		Logger::error("server error: could not add certificate or key file");
		ERR_print_errors_fp(stderr);
		return (false);
	}

	/**
	 * Ajour des ciphers à utiliser pour le chiffrement des données.
	 */
	if (SSL_CTX_set_cipher_list(this->_ssl.ctx, this->_ssl.ciphers.c_str()) == 0)
	{
		Logger::error("server error: could not set ciphers");
		ERR_print_errors_fp(stderr);
		return (false);
	}
	return (true);
}

SSL_CTX	*ServerConfig::getSSLCTX(void) const
{
	return (this->_ssl.ctx);
}

void	ServerConfig::addNames(const std::vector<std::string> &name)
{
	for (std::vector<std::string>::const_iterator it = name.begin(); it != name.end(); it++)
	{
		this->addName(*it);
	}
}

void	ServerConfig::addName(const std::string &name)
{
	if (name.find(':') != std::string::npos)
		Logger::error("server error: invalid server name: " + name);
	else {
		this->_server_name.push_back((struct wbs_server_name){.name = name});
	}
}

const wbs_server_names	&ServerConfig::getNames(void) const
{
	return (this->_server_name);
}

bool	ServerConfig::evalName(const std::string &name) const
{
	for (size_t i = 0; i < this->_server_name.size(); ++i) {
		if (this->_server_name[i].name == name) {
			return (true);
		}
	}
	return (false);
}

void	ServerConfig::print(std::ostream &os) const
{
	os << B_BLUE << "Config: " << RESET;
	if (this->_server->getDefault() == this)
		os << B_GREEN << "default" << RESET;
	os << "\n";
	os << B_CYAN << "Name: " << RESET;
	if (this->_server_name.empty())
		os << I_YELLOW << "Any" << RESET;
	for (wbs_server_names::const_iterator it = this->_server_name.begin(); it != this->_server_name.end(); it++)
	{
		os << it->name;
		os << " ";
	}
	os << "\n";
	os << B_ORANGE << "Default router: " << RESET << "\n";
	os << *this->_default;
}
