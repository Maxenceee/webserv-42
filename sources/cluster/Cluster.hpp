/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:47:56 by mgama             #+#    #+#             */
/*   Updated: 2024/02/27 11:57:34 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include "webserv.hpp"
#include "parser/Parser.hpp"
#include "server/Server.hpp"
#include "server/ServerConfig.hpp"

class Parser;

typedef std::vector<Server*>	v_servers;

class Cluster
{
private:
	Parser					*parser;
	std::vector<Server*>	_servers;

public:
	Cluster(void);
	~Cluster();

	static bool		exit;

	int		start(void);

	void	parse(const char *configPath);
	void	initConfigs(std::vector<ServerConfig *> configs);
	Server	*addConfig(ServerConfig *config);
};

#endif /* CLUSTER_HPP */