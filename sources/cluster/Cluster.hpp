/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:47:56 by mgama             #+#    #+#             */
/*   Updated: 2024/01/11 19:59:30 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "parser/Parser.hpp"
#include "server/Server.hpp"

class Cluster
{
private:
	Parser					parser;
	std::vector<Server>		_servers;

public:
	Cluster(const char *configPath);
	~Cluster();
};

