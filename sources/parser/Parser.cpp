/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:18:32 by mgama             #+#    #+#             */
/*   Updated: 2024/02/02 21:30:19 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Parser.hpp"

#define PARSER_ERR		B_RED"parser error: invalid file path"RESET

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
	if (!this->file)
		return (std::cerr << "parser error: Could not open file " << file_name << std::endl, EXIT_FAILURE);

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
}

void	Parser::processInnerLines(const std::string &lineRaw, std::string &chunkedLine, std::string &parent, int &countOfParents)
{
	std::vector<std::string> innerLines = split(lineRaw, '\n');

	for (std::vector<std::string>::iterator innerIt = innerLines.begin(); innerIt != innerLines.end(); ++innerIt) {
		std::string line = *innerIt;
		trim(line);
		chunkedLine.empty() ? line : chunkedLine + " " + line;

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
			// std::cout << B_YELLOW << k ey << RESET << "\t" << val << "\t" << B_GREEN << parent << RESET << std::endl;
			this->switchConfigDirectives(key, val, parent);
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
			// std::cout << B_RED << key << RESET << "\t" << val << "\t" << B_GREEN << parent << RESET << std::endl;
			this->switchConfigDirectives(key, val, parent);
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

		// Block transformation
		std::vector<std::string> innerLines = split(lineRaw, '\n');
		for (std::vector<std::string>::iterator innerIt = innerLines.begin(); innerIt != innerLines.end(); ++innerIt) {
			std::string line = *innerIt;
			trim(line);
			this->processInnerLines(line, chunkedLine, parent, countOfParents);
		}
	}
}

void	Parser::throwError(const std::string key, const std::string val)
{
	std::string	tmp(B_RED"parser error: invalid directive found: ");
	tmp += key + " " + val;
	tmp += RESET;
	throw std::invalid_argument(tmp.c_str());
}

void	Parser::switchConfigDirectives(const std::string key, const std::string val, const std::string parent)
{
	if (!this->new_server && key != "server")
		this->throwError(key, val);
	else if (key == "server") {
		this->new_server = this->cluster.newServer();
		this->tmp_router = &this->new_server->getDefaultHandler();
	}
	else if (key == "location")
		this->createNewRouter(key, val);
	else
	{
		if (parent == "server")
			this->tmp_router = &this->new_server->getDefaultHandler();
		this->addRule(key, val, parent);
	}
}

void	Parser::createNewRouter(const std::string key, const std::string val)
{
	std::vector<std::string> tokens = split(val, ' ');
	if (tokens.size() > 2 || tokens.size() < 1)
		this->throwError(key, val);
	bool strict = false;
	if (tokens.size() == 2 && tokens[0] == "=")
		strict = true;
	else if (tokens.size() == 2)
		this->throwError(key, val);
	std::string	path = tokens[tokens.size() - 1];
	this->tmp_router = new Router(*this->new_server, path, this->new_server->getDefaultHandler().getRoot(), strict);
	this->new_server->use(this->tmp_router);
}

void	Parser::addRule(const std::string key, const std::string val, const std::string parent)
{
	/**
	 * Directive Listen
	 */
	if (key == "listen" && parent != "server")
		this->throwError(key, val);
	else if (key == "listen") {
		this->new_server->setPort(std::atoi(val.c_str()));
		return ;
	}

	/**
	 * Directive server_name
	 */
	if (key == "server_name" && parent != "server")
		this->throwError(key, val);
	else if (key == "server_name") {
		this->new_server->setName(val);
		return ;
	}

	/**
	 * Directive root/alias
	 */
	if (key == "root") {
		this->tmp_router->setRoot(val);
		return ;
	}
	else if (key == "alias") {
		this->tmp_router->setAlias(val);
		return ;
	}

	/**
	 * Directive index
	 */
	if (key == "index") {
		std::string index = split(val, ' ')[0];
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
		/*
		 * TODO:
		 * verifier si val contient une espace avant de split pour eviter 
		 * de split pour rien
		 */
		std::cout << "return " << key << " " << val << std::endl;
		std::vector<std::string> tokens = split(val, ' ');
		if (tokens.size() == 2) {
			status = std::atoi(tokens[0].c_str());
			loc = tokens[1];
		}
		else if (tokens.size() > 2)
			this->throwError(key, val);
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
	 * Default
	 */
	this->throwError(key, val);
}
