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

class Parser
{
private:

	std::fstream					file;
	std::vector<std::string>		buffer;
	
	int		open_and_read_file(const char *file_name);

public:
	Parser(void);
	~Parser(void);

	void	parse(const char *configPath);
};
