/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:53 by mgama             #+#    #+#             */
/*   Updated: 2025/01/04 22:38:34 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "webserv.hpp"

class Logger
{
private:
	static bool				_debug;
	static pthread_mutex_t	_loggerMutex;
	static bool				_initiated;

	static bool				aquireMutex(void);
	static void				releaseMutex(void);

public:
	static void init(void);
	static void destroy(void);

	static void	print(const char *msg, const std::string &color = RESET);
	static void	print(const std::string &msg, const std::string &color = RESET);

	static void	info(const char *msg);
	static void	info(const std::string &msg);

	static void	warning(const char *msg);
	static void	warning(const std::string &msg);

	static void	error(const char *msg);
	static void	error(const std::string &msg);

	static void	perror(const char *msg);
	static void	perror(const std::string &msg);

	static void	pherror(const char *msg);
	static void	pherror(const std::string &msg);

	static void	debug(const char *msg, const std::string &color = RESET);
	static void	debug(const std::string &msg, const std::string &color = RESET);

	static void setDebug(bool debug);
	static bool isDebug(void);
};

#endif /* LOGGER_HPP */