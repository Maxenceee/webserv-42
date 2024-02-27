/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:56 by mgama             #+#    #+#             */
/*   Updated: 2024/02/27 21:19:40 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

bool Logger::_debug = false;

void	Logger::print(const std::string &msg, const std::string &color)
{
	struct tm	*tm;
	time_t rawtime;
	char buf[32];

	time(&rawtime);
	tm = localtime (&rawtime);
	int ret = strftime(buf, 32, "%T", tm);
	buf[ret] = '\0';
	std::cout << CYAN << "[" << buf << "] " << RESET;
	std::cout << color << msg << RESET;
	std::cout << "\n";
}

void	Logger::info(const std::string &msg)
{
	struct tm	*tm;
    time_t rawtime;
    char buf[32];

    time(&rawtime);
    tm = localtime (&rawtime);
    int ret = strftime(buf, 32, "%T", tm);
    buf[ret] = '\0';
	std::cout << CYAN << "[" << buf << "] " << RESET;
	std::cout << YELLOW << msg << RESET;
	std::cout << "\n";
}

void	Logger::warning(const std::string &msg, const std::string &color)
{
	struct tm	*tm;
    time_t rawtime;
    char buf[32];

    time(&rawtime);
    tm = localtime (&rawtime);
    int ret = strftime(buf, 32, "%T", tm);
    buf[ret] = '\0';
	std::cerr << CYAN << "[" << buf << "] " << RESET;
	std::cerr << color << msg << RESET;
	std::cerr << "\n";
}

void	Logger::error(const std::string &msg, const std::string &color)
{
	struct tm	*tm;
    time_t rawtime;
    char buf[32];

    time(&rawtime);
    tm = localtime (&rawtime);
    int ret = strftime(buf, 32, "%T", tm);
    buf[ret] = '\0';
	std::cerr << CYAN << "[" << buf << "] " << RESET;
	std::cerr << color << msg << RESET;
	std::cerr << "\n";
}

void	Logger::debug(const std::string &msg, const std::string &color)
{
	if (!Logger::_debug)
		return ;

	struct tm	*tm;
    time_t rawtime;
    char buf[32];

    time(&rawtime);
    tm = localtime (&rawtime);
    int ret = strftime(buf, 32, "%T", tm);
    buf[ret] = '\0';
	std::cout << CYAN << "[" << buf << "] " << RESET;
	std::cout << color << msg << RESET;
	std::cout << "\n";
}
