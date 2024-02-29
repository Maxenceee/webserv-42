/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:17:45 by mgama             #+#    #+#             */
/*   Updated: 2024/02/28 23:03:58 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include "webserv.hpp"
#include "cluster/Cluster.hpp"
#include "server/Server.hpp"
#include "server/ServerConfig.hpp"
#include "routes/Router.hpp"
#include "logger/Logger.hpp"

class Cluster;
class Router;

class Parser
{
private:
	Cluster			&cluster;
	std::fstream	file;
	std::string		buffer;

	ServerConfig	*new_server;
	Router			*tmp_router;

	std::vector<ServerConfig *>	configs;
	
	int		open_and_read_file(const char *file_name);

	void	extract(const std::string &conf);
	void	processInnerLines(const std::string &lineRaw, std::string &chunkedLine, std::string &parent, int &countOfParents);

	void	switchConfigDirectives(const std::string key, const std::string val, const std::string parent, const std::string raw_line);
	void	createNewRouter(const std::string key, const std::string val, const std::string parent, const std::string raw_line);
	void	addRule(const std::string key, const std::string val, const std::string parent, const std::string raw_line);
	bool	isValidModifier(const std::string &modifier) const;

	void	throwError(const std::string key, const std::string val, const std::string raw_line = "");

public:
	Parser(Cluster &c);
	~Parser(void);

	void	parse(const char *configPath);
};

#endif /* PARSER_HPP */