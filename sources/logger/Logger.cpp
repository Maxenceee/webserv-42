/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:56 by mgama             #+#    #+#             */
/*   Updated: 2024/02/23 20:56:39 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

Logger::Logger(const bool debug): _debug(debug)
{
}

void	Logger::print(const std::string &msg, const bool error = false)
{
	struct tm	*tm;
    time_t rawtime;
    char buf[32];

    time(&rawtime);
    tm = localtime (&rawtime);
    int ret = strftime(buf, 32, "%T", tm);
    buf[ret] = '\0';
	if (error) {
		std::cerr << CYAN << "[" << buf << "] " << RESET;
		std::cerr << B_RED << msg << RESET;
		std::cerr << "\n";
	} else {
		std::cout << CYAN << "[" << buf << "] " << RESET;
		std::cout << B_BLUE << msg << RESET;
		std::cout << "\n";
	}
}
