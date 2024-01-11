/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:17:45 by mgama             #+#    #+#             */
/*   Updated: 2024/01/11 20:00:07 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"

class Parser
{
private:

	std::fstream					file;
	std::vector<std::string>		buffer;
	
	int		open_and_read_file(std::string file_name);

public:
	Parser(void);
	~Parser(void);

	void	parse(const std::string configPath);
};
