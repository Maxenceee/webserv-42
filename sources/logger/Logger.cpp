/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:56 by mgama             #+#    #+#             */
/*   Updated: 2024/11/17 14:59:56 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

bool Logger::_debug = false;
bool Logger::_initiated = false;
pthread_mutex_t Logger::_loggerMutex;

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
	/**
	 * Initialisation du mutex pour eviter les conflits d'affichage
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
		/**
		 * If the logger is not initiated, we need to init it
		 */
		Logger::init();
	}
	return (pthread_mutex_lock(&Logger::_loggerMutex) == 0);
}

void	Logger::releaseMutex(void)
{
	pthread_mutex_unlock(&Logger::_loggerMutex);
}

void	Logger::print(const std::string &msg, const std::string &color)
{
	struct tm	*tm;
	time_t rawtime;
	char buf[32];

	Logger::aquireMutex();
	time(&rawtime);
	tm = localtime (&rawtime);
	int ret = strftime(buf, 32, "%T", tm);
	buf[ret] = '\0';
	std::cout << CYAN << "[" << buf << "] " << RESET;
	std::cout << color << msg << RESET;
	std::cout << std::endl;
	Logger::releaseMutex();
}

void	Logger::info(const std::string &msg)
{
	struct tm	*tm;
	time_t rawtime;
	char buf[32];

	Logger::aquireMutex();
	time(&rawtime);
	tm = localtime (&rawtime);
	int ret = strftime(buf, 32, "%T", tm);
	buf[ret] = '\0';
	std::cout << CYAN << "[" << buf << "] " << RESET;
	std::cout << YELLOW << msg << RESET;
	std::cout << std::endl;
	Logger::releaseMutex();
}

void	Logger::warning(const std::string &msg, const std::string &color)
{
	struct tm	*tm;
	time_t rawtime;
	char buf[32];

	Logger::aquireMutex();
	time(&rawtime);
	tm = localtime (&rawtime);
	int ret = strftime(buf, 32, "%T", tm);
	buf[ret] = '\0';
	std::cerr << CYAN << "[" << buf << "] " << RESET;
	std::cerr << color << WBS_PREFIX << msg << RESET;
	std::cerr << std::endl;
	Logger::releaseMutex();
}

void	Logger::error(const std::string &msg, const std::string &color)
{
	struct tm	*tm;
	time_t rawtime;
	char buf[32];

	Logger::aquireMutex();
	time(&rawtime);
	tm = localtime (&rawtime);
	int ret = strftime(buf, 32, "%T", tm);
	buf[ret] = '\0';
	std::cerr << CYAN << "[" << buf << "] " << RESET;
	std::cerr << color << WBS_PREFIX << msg << RESET;
	std::cerr << std::endl;
	Logger::releaseMutex();
}

void	Logger::perror(const std::string &msg, const std::string &color)
{
	struct tm	*tm;
	time_t rawtime;
	char buf[32];

	Logger::aquireMutex();
	time(&rawtime);
	tm = localtime (&rawtime);
	int ret = strftime(buf, 32, "%T", tm);
	buf[ret] = '\0';
	std::cerr << CYAN << "[" << buf << "] " << RESET;
	std::cerr << color << WBS_PREFIX << msg << ": " << strerror(errno) << RESET;
	std::cerr << std::endl;
	Logger::releaseMutex();
}

void	Logger::debug(const std::string &msg, const std::string &color)
{
	if (!Logger::_debug)
		return ;

	struct tm	*tm;
	time_t rawtime;
	char buf[32];

	Logger::aquireMutex();
	time(&rawtime);
	tm = localtime (&rawtime);
	int ret = strftime(buf, 32, "%T", tm);
	buf[ret] = '\0';
	std::cout << CYAN << "[" << buf << "] " << RESET;
	std::cout << color << msg << RESET;
	std::cout << std::endl;
	Logger::releaseMutex();
}
