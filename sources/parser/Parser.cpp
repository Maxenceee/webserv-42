/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:18:32 by mgama             #+#    #+#             */
/*   Updated: 2024/02/28 19:27:49 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Parser.hpp"

#define PARSER_ERR		RED"parser error: invalid file path"RESET

Parser::Parser(Cluster &c): cluster(c)
{
	this->new_server = NULL;
}

Parser::~Parser(void)
{
}

int		Parser::open_and_read_file(const char *file_name)
{
	std::string		line;
	std::vector<std::string>	tmp;
	
	this->file.open(file_name, std::ios::in);
	if (!this->file) {
		Logger::error("parser error: Could not open file " + std::string(file_name), RESET);
		return (EXIT_FAILURE);
	}

	while (getline(this->file, line))
	{
		tmp.push_back(line);
	}
	this->buffer = join(tmp, "\n");
	return (EXIT_SUCCESS);
}

void	Parser::parse(const char *configPath)
{
	if (!isFile(configPath))
		throw std::invalid_argument(PARSER_ERR);
	this->open_and_read_file(configPath);
	this->extract(this->buffer);
	this->cluster.initConfigs(this->configs);
}

void	Parser::processInnerLines(const std::string &lineRaw, std::string &chunkedLine, std::string &parent, int &countOfParents)
{
	std::vector<std::string> innerLines = split(lineRaw, '\n');

	for (std::vector<std::string>::iterator innerIt = innerLines.begin(); innerIt != innerLines.end(); ++innerIt) {
		std::string line = *innerIt;
		trim(line);
		line = chunkedLine.empty() ? line : chunkedLine + " " + line;

		// Object opening line
		if (line[line.length() - 1] == '{') {
			chunkedLine = "";
			std::string key = line.substr(0, line.length() - 1);
			trim(key);
			std::vector<std::string> tokens = split(key, ' ');
			key = tokens[0];
			trim(key);
			shift(tokens);
			std::string val = join(tokens, " ");
			if (!parent.empty()) {
				parent += '.' + key;
			} else {
				parent = key;
			}
			Logger::debug(YELLOW + key + RESET + "\t" + val + "\t" + GREEN + parent + RESET);
			this->switchConfigDirectives(key, val, parent, line);
		}
		// Standard property line
		else if (line[line.length() - 1] == ';') {
			chunkedLine = "";
			std::istringstream iss(line);
			std::vector<std::string> tokens;
			std::copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(), std::back_inserter(tokens));
			std::string key = tokens[0];
			tokens.erase(tokens.begin());
			std::string val = join(tokens, " ");
			trim(key);
			trim(val);
			if (key[key.length() - 1] == ';') {
				pop(key);
			}
			pop(val);
			Logger::debug(RED + key + RESET + "\t" + val + "\t" + GREEN + parent + RESET);
			this->switchConfigDirectives(key, val, parent, line);
		}
		// Object closing line
		else if (line[line.length() - 1] == '}') {
			chunkedLine = "";
			std::vector<std::string> parentTokens = split(parent, '.');
			if (countOfParents > 0 && isdigit(parentTokens.back()[0])) {
				parentTokens.pop_back();
				countOfParents -= 1;
			}
			parentTokens.pop_back();
			parent = join(parentTokens, ".");
		}
		// Line may not contain '{' ';' '}' symbols at the end
		else {
			chunkedLine = line;
		}
	}
}

void	Parser::extract(const std::string &conf)
{
	std::vector<std::string> lines = split(conf, '\n');
	std::string parent = "";
	std::string chunkedLine = "";
	int countOfParents = 0;

	for (std::vector<std::string>::iterator lineIt = lines.begin(); lineIt != lines.end(); ++lineIt) {
		std::string lineRaw = *lineIt;
		trim(lineRaw);

		// If line is blank line or is comment, do not process it
		if (lineRaw.empty() || lineRaw[0] == '#') continue;

		// Line can contain comments, we need to remove them
		lineRaw = split(lineRaw, '#')[0];
		trim(lineRaw);
		this->processInnerLines(lineRaw, chunkedLine, parent, countOfParents);
	}
}

void	Parser::throwError(const std::string key, const std::string val, const std::string raw_line)
{
	std::string	tmp(B_RED"parser error: invalid directive found");
	if (raw_line.size() > 0) {
		tmp += RESET;
		tmp += "\n\t" + raw_line;
		tmp += "\n"B_GREEN"\t^\n"RESET;
		tmp += "1 error generated.";
	}
	else
		tmp += ": " + key + " " + val;
	tmp += RESET;
	throw std::invalid_argument(tmp.c_str());
}

void	Parser::switchConfigDirectives(const std::string key, const std::string val, const std::string parent, const std::string raw_line)
{
	if (!this->new_server && key != "server")
		this->throwError(key, val, raw_line);
	else if (key == "server") {
		// On empeche l'imbrication de blocs server
		if (parent != "server")
			this->throwError(key, val, raw_line);
		this->new_server = new ServerConfig();
		this->configs.push_back(this->new_server);
		this->tmp_router = &this->new_server->getDefaultHandler();
	}
	else if (key == "location") {
		// On empeche l'imbrication des locations
		if (parent != "server.location")
			this->throwError(key, val, raw_line);
		this->createNewRouter(key, val, raw_line);
	}
	else
	{
		if (parent == "server")
			this->tmp_router = &this->new_server->getDefaultHandler();
		if (val.empty())
			this->throwError(key, val, raw_line);
		this->addRule(key, val, parent, raw_line);
	}
}

void	Parser::createNewRouter(const std::string key, const std::string val, const std::string raw_line)
{
	struct s_Router_Location	location;

	std::vector<std::string> tokens = split(val, ' ');
	if (tokens.size() > 2 || tokens.size() < 1)
		this->throwError(key, val, raw_line);
	if (tokens.size() == 2 && this->isValidModifier(tokens[0])) {
		location.modifier = tokens[0];
		if (location.modifier == "=")
			location.strict = true;
	}
	else if (tokens.size() == 2)
		this->throwError(key, val, raw_line);
	location.path = tokens[tokens.size() - 1];
	this->tmp_router = new Router(*this->new_server, location, this->new_server->getDefaultHandler().getRoot());
	this->new_server->use(this->tmp_router);
}

void	Parser::addRule(const std::string key, const std::string val, const std::string parent, const std::string raw_line)
{
	/**
	 * Directive Listen
	 */
	if (key == "listen" && parent != "server")
		this->throwError(key, val);
	else if (key == "listen") {
		// check if there is a column in the value
		if (val.find(':') == std::string::npos) {
			this->new_server->setPort(std::atoi(val.c_str()));
		} else {
			std::vector<std::string> tokens = split(val, ':');
			if (tokens.size() == 2) {
				this->new_server->setAddress(tokens[0]);
				this->new_server->setPort(std::atoi(tokens[1].c_str()));
			}
			else
				this->throwError(key, val);
		}
		return ;
	}

	/**
	 * Directive server_name
	 */
	if (key == "server_name" && parent != "server")
		this->throwError(key, val);
	else if (key == "server_name") {
		std::vector<std::string> names = split(val, ' ');
		this->new_server->setNames(names);
		return ;
	}

	/**
	 * Directive root/alias
	 */
	if (key == "root") {
		this->tmp_router->setRoot(val);
		return ;
	}
	else if (key == "alias" && parent == "server.location") {
		this->tmp_router->setAlias(val);
		return ;
	} else if (key == "alias") {
		this->throwError(key, val);
		return ;
	}

	/**
	 * Directive index
	 */
	if (key == "index") {
		std::vector<std::string> index = split(val, ' ');
		this->tmp_router->setIndex(index);
		return ;
	}

	/**
	 * Directive autoindex
	 */
	if (key == "autoindex") {
		if (val == "on")
			this->tmp_router->setAutoIndex(true);
		return ;
	}

	/**
	 * Directive return
	 */
	if (key == "return") {
		int status = 302;
		std::string loc = val;
		if (val.find(' ') != std::string::npos) {
			std::vector<std::string> tokens = split(val, ' ');
			if (tokens.size() == 2) {
				status = std::atoi(tokens[0].c_str());
				loc = tokens[1];
			}
			else if (tokens.size() > 2)
				this->throwError(key, val);
		} else if (isDigit(val)) {
			status = std::atoi(val.c_str());
			loc = "";
		}
		if (Response::http_codes.count(status) == 0) {
			Logger::warning("parser warning: invalid status code, this may cause unexpected behavior.");
		}
		this->tmp_router->setRedirection(loc, status);
		return ;
	}

	/**
	 * Directive allow_methods
	 */
	if (key == "allow_methods") {
		std::vector<std::string> tokens = split(val, ' ');
		for (size_t i = 0; i < tokens.size(); i++) {
			this->tmp_router->allowMethod(tokens[i]);
		}
		return ;
	}

	/**
	 * Directive error_page
	 */
	if (key == "error_page") {
		std::vector<std::string> tokens = split(val, ' ');
		if (tokens.size() < 2)
			this->throwError(key, val);
		for (size_t i = 0; i < tokens.size() - 1; i++) {
			this->tmp_router->setErrorPage(std::atoi(tokens[i].c_str()), tokens[tokens.size() - 1]);
		}
		return ;
	}

	/**
	 * Directive client_max_body_size
	 */
	if (key == "client_max_body_size") {
		this->tmp_router->setClientMaxBodySize(val);
		return ;
	}

	/**
	 * Directive cgi_extension
	 */

	if (key == "cgi") {
		if (val == "on")
			this->tmp_router->enableCGI();
		return ;
	}

	/**
	 * Directive fastcgi_pass
	 */
	if (key == "fastcgi_pass") {
		// std::vector<std::string> tokens = split(val, ' ');
		// if (tokens.size() < 2)
		// 	this->throwError(key, val);
		this->tmp_router->setCGI(val);
		return ;
	}

	/**
	 * Directive fastcgi_param
	 */
	if (key == "fastcgi_param") {
		std::vector<std::string> tokens = split(val, ' ');
		if (tokens.size() < 2)
			this->throwError(key, val);
		this->tmp_router->addCGIParam(tokens[0], tokens[1]);
	}
	/**
	 * Default
	 */
	this->throwError(key, val, raw_line);
}

bool	Parser::isValidModifier(const std::string &modifier) const
{
	return (modifier == "=" || modifier == "~" || modifier == "~*" || modifier == "^~");
}
