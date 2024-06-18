/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:56 by mgama             #+#    #+#             */
/*   Updated: 2024/06/18 23:40:40 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

bool Logger::_debug = false;

void	Logger::init(void)
{
	struct tm	*tm;
    time_t rawtime;
    char buf[32];

    time(&rawtime);
    tm = localtime (&rawtime);
    int ret = strftime(buf, 32, "%T", tm);
    buf[ret] = '\0';
	std::stringstream ss;
	ss << CYAN << "[" << buf << "] " << RESET;
	ss  << WBS_PREFIX << "Starting server: New logger session" << RESET;
	std::cerr << ss.str() << std::endl;
	std::cout << ss.str() << std::endl;
}

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
	std::cout << std::endl;
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
	std::cout << std::endl;
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
	std::cerr << color << WBS_PREFIX << msg << RESET;
	std::cerr << std::endl;
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
	std::cerr << color << WBS_PREFIX << msg << RESET;
	std::cerr << std::endl;
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
	std::cout << std::endl;
}
