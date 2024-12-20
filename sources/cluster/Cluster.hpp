/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:47:56 by mgama             #+#    #+#             */
/*   Updated: 2024/12/20 15:16:54 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include "webserv.hpp"
#include "parser/Parser.hpp"
#include "server/Server.hpp"
#include "server/ServerConfig.hpp"
#include "client/Client.hpp"
#include "proxy/threads/ThreadPool.hpp"

class Parser;

typedef std::vector<Server*>	wsb_v_servers_t;

enum wbs_polltype {
	WBS_POLL_SERVER	= 0x00,	// A server socket
	WBS_POLL_CLIENT	= 0x01	// A client socket
};

enum wbs_pollclientstatus {
	WBS_POLL_CLIENT_OK			= 0x00,
	WBS_POLL_CLIENT_DISCONNECT	= 0x01,
	WBS_POLL_CLIENT_CLOSED		= 0x02,
	WBS_POLL_CLIENT_ERROR		= 0x03
};

struct wbs_pollclient {
	enum wbs_polltype	type;
	void				*data;
};

class Cluster
{
private:
	Parser				*parser;
	wsb_v_servers_t		_servers;

public:
	Cluster(void);
	~Cluster();

	static bool			exit;
	static bool			kill;

	static ThreadPool	pool;

	int		start(void);

	static void initializePool(size_t numThreads) {
		pool.~ThreadPool();
		new(&pool) ThreadPool(numThreads);
	}

	void	parse(const std::string &configPath);
	void	initConfigs(std::vector<ServerConfig *> &configs);
	Server	*addConfig(ServerConfig *config);
};

#endif /* CLUSTER_HPP */
