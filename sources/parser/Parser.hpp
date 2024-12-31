/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:17:45 by mgama             #+#    #+#             */
/*   Updated: 2024/12/31 19:33:08 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
#define PARSER_HPP

#include "webserv.hpp"
#include "cluster/Cluster.hpp"
#include "server/Server.hpp"
#include "server/ServerConfig.hpp"
#include "logger/Logger.hpp"

class Cluster;
class Router;
class ServerConfig;

class Parser
{
private:
	Cluster						&cluster;
	std::fstream				file;
	std::vector<std::string>	buffer;

	ServerConfig	*new_server;
	Router			*tmp_router;

	std::vector<ServerConfig *>	configs;
	
	int		open_and_read_file(const std::string &file_name);

	void	extract(void);
	void	processInnerLines(std::vector<std::string> &tokens, std::vector<std::string> &parent);

	void	switchConfigDirectives(const std::string &key, const std::string &val, const std::string &context, const std::string &raw_line);
	void	createNewRouter(const std::string &key, const std::string &val, const std::string &context, const std::string &raw_line);
	void	addRule(const std::string &key, const std::string &val, const std::string &context, const std::string &raw_line);
	bool	isValidModifier(const std::string &modifier) const;

	void	throwError(const std::string &raw_line, const int pos = 0);
	void	throwError(const std::string &raw_line, const std::string &message, const int pos = 0);

	void	minmaxArgs(const std::string &raw_line, const size_t key_length, const std::string &val, const std::vector<std::string> &valtokens, const size_t min, const size_t max = 0);
	bool	onoffArgs(const std::string &raw_line, const size_t key_length, const std::string &val, const std::vector<std::string> &valtokens);

public:
	Parser(Cluster &c);
	~Parser(void);

	void	parse(const std::string &configPath);
};

#endif /* PARSER_HPP */