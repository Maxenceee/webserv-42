/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:17:45 by mgama             #+#    #+#             */
/*   Updated: 2024/01/12 12:05:54 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "cluster/Cluster.hpp"
#include "server/Server.hpp"
#include "routes/Router.hpp"

class Cluster;

class Parser
{
private:
	Cluster			&cluster;
	std::fstream	file;
	std::string		buffer;

	Server	*new_server;
	Router	*tmp_router;
	
	int		open_and_read_file(const char *file_name);

	void	extract(const std::string &conf);
	void	processInnerLines(const std::string &lineRaw, std::string &chunkedLine, std::string &parent, int &countOfParentsThatAreArrays);

	void	switchConfigDirectives(const std::string key, const std::string val, const std::string parent);
	void	createNewRouter(const std::string key, const std::string val);
	void	addRule(const std::string key, const std::string val, const std::string parent);

	void	throwError(const std::string key, const std::string val);

public:
	Parser(Cluster &c);
	~Parser(void);

	void	parse(const char *configPath);
};
