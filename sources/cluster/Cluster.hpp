/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:47:56 by mgama             #+#    #+#             */
/*   Updated: 2024/04/14 17:15:40 by mgama            ###   ########.fr       */
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

enum polltype {
	POLL_SERVER = 0x00,
	POLL_CLIENT = 0x01
};

enum pollclientstatus {
	POLL_CLIENT_OK = 0x00,
	POLL_CLIENT_DISCONNECT = 0x01,
	POLL_CLIENT_CLOSED = 0x02,
	POLL_CLIENT_ERROR = 0x03
};

struct pollclient {
	enum polltype	type;
	void			*data;
};

class Cluster
{
private:
	Parser					*parser;
	std::vector<Server*>	_servers;
	// std::vector<pollfd>		poll_fds;

public:
	Cluster(void);
	~Cluster();

	static bool		exit;

	int		start(void);

	void	parse(const char *configPath);
	void	initConfigs(std::vector<ServerConfig *> &configs);
	Server	*addConfig(ServerConfig *config);
};

#endif /* CLUSTER_HPP */