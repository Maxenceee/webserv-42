/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:56 by mgama             #+#    #+#             */
/*   Updated: 2024/12/20 15:50:36 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

bool Logger::_debug = false;
bool Logger::_initiated = false;
pthread_mutex_t Logger::_loggerMutex;

static void	displayDate(std::ostream& os)
{
    struct tm *tm;
    time_t rawtime;
    char buf[32];

    time(&rawtime);
    tm = localtime(&rawtime);
    int ret = strftime(buf, 32, "%T", tm);
    buf[ret] = '\0';
    os << CYAN << "[" << buf << "] " << RESET;
}

void	Logger::init(void)
{
	displayDate(std::cout);
	std::cout << WBS_PREFIX << "Starting server: New logger session" << RESET << std::endl;
	displayDate(std::cerr);
	std::cerr << WBS_PREFIX << "Starting server: New logger session" << RESET << std::endl;
	/**
	 * Initialisation du mutex pour éviter les conflits d'affichage
	 */
	pthread_mutex_init(&Logger::_loggerMutex, NULL);
	std::atexit(Logger::destroy);
	Logger::_initiated = true;
}

void	Logger::destroy(void)
{
	if (!Logger::_initiated)
		return ;

	pthread_mutex_destroy(&Logger::_loggerMutex);
}

bool	Logger::aquireMutex(void)
{
	if (!Logger::_initiated)
	{
		throw std::runtime_error("Logger should be initiated before use (Logger::init)");
	}
	return (pthread_mutex_lock(&Logger::_loggerMutex) == 0);
}

void	Logger::releaseMutex(void)
{
	pthread_mutex_unlock(&Logger::_loggerMutex);
}

void	Logger::print(const std::string &msg, const std::string &color)
{
	Logger::aquireMutex();
	displayDate(std::cout);
	std::cout << color << msg << RESET;
	std::cout << std::endl;
	Logger::releaseMutex();
}

void	Logger::info(const std::string &msg)
{
	Logger::aquireMutex();
	displayDate(std::cout);
	std::cout << YELLOW << msg << RESET;
	std::cout << std::endl;
	Logger::releaseMutex();
}

void	Logger::warning(const std::string &msg, const std::string &color)
{
	Logger::aquireMutex();
	displayDate(std::cerr);
	std::cerr << color << WBS_PREFIX << msg << RESET;
	std::cerr << std::endl;
	Logger::releaseMutex();
}

void	Logger::error(const std::string &msg, const std::string &color)
{
	Logger::aquireMutex();
	displayDate(std::cerr);
	std::cerr << color << WBS_PREFIX << msg << RESET;
	std::cerr << std::endl;
	Logger::releaseMutex();
}

void	Logger::perror(const std::string &msg, const std::string &color)
{
	Logger::aquireMutex();
	displayDate(std::cerr);
	std::cerr << color << WBS_PREFIX << msg << ": " << strerror(errno) << RESET;
	std::cerr << std::endl;
	Logger::releaseMutex();
}

void	Logger::debug(const std::string &msg, const std::string &color)
{
	if (!Logger::_debug)
		return ;

	Logger::aquireMutex();
	displayDate(std::cout);
	std::cout << color << msg << RESET;
	std::cout << std::endl;
	Logger::releaseMutex();
}

void	Logger::setDebug(bool debug)
{
	Logger::_debug = debug;
}

bool	Logger::isDebug(void)
{
	return (Logger::_debug);
}