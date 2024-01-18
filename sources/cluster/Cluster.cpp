/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:48:08 by mgama             #+#    #+#             */
/*   Updated: 2024/01/18 14:15:34 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"

Cluster::Cluster(const char *configPath)
{
	this->parser = new Parser(*this);
	this->parser->parse(configPath);
	for (v_servers::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
		std::cout << *(*it) << std::endl;
}

Cluster::~Cluster()
{
	for (v_vservers::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
		delete *it;
	delete this->parser;
	std::cout << B_GREEN << "Shutting down webserv" << RESET << std::endl;
}

Server	*Cluster::newServer(void)
{
	Server	*server = new Server;
	this->_servers.push_back(server);
	return (server);
}