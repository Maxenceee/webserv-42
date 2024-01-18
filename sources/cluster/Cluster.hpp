/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:47:56 by mgama             #+#    #+#             */
/*   Updated: 2024/01/18 14:15:37 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include "webserv.hpp"
#include "parser/Parser.hpp"
#include "server/Server.hpp"

class Parser;

typedef std::vector<Server*>	v_servers;

class Cluster
{
private:
	Parser					*parser;
	std::vector<Server*>	_servers;

public:
	Cluster(const char *configPath);
	~Cluster();

	Server	*newServer(void);
};

#endif /* CLUSTER_HPP */