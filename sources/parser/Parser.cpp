/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:18:32 by mgama             #+#    #+#             */
/*   Updated: 2024/01/16 18:09:09 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Parser.hpp"

#define PARSER_ERR		B_RED"parser error: invalid file path"RESET

Parser::Parser(void)
{
}

Parser::~Parser(void)
{
}

int		Parser::open_and_read_file(const char *file_name)
{
	std::string		line;
	
	this->file.open(file_name, std::ios::in);
	if (!this->file)
		return (std::cerr << "parser error: Could not open file " << file_name << std::endl, EXIT_FAILURE);

	while (getline(this->file, line))
	{
		this->buffer.push_back(line);
	}
	return (EXIT_SUCCESS);
}

void	Parser::parse(const char *configPath)
{
	if (!isFile(configPath))
		throw std::invalid_argument(PARSER_ERR);
	this->open_and_read_file(configPath);

	for (std::vector<std::string>::iterator it = this->buffer.begin(); it != this->buffer.end(); it++)
	{
		std::cout << *it << std::endl;
		std::string line = it->replace(it->find('\t'), it->length(), "");
		line = trim(line, ' ');
		if (line.empty() || line[0] == '#')
			continue;
	}
}
