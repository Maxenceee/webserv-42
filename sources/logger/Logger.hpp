/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:53 by mgama             #+#    #+#             */
/*   Updated: 2024/11/17 15:00:14 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "webserv.hpp"

class Logger
{
private:
	static pthread_mutex_t	_loggerMutex;
	static bool				_initiated;

	static bool				aquireMutex(void);
	static void				releaseMutex(void);

public:
	static bool	_debug;

	static void init(void);
	static void destroy(void);
	static void	print(const std::string &msg, const std::string &color = RESET);
	static void	info(const std::string &msg);
	static void	warning(const std::string &msg, const std::string &color = B_ORANGE);
	static void	error(const std::string &msg, const std::string &color = B_RED);
	static void	perror(const std::string &msg, const std::string &color = B_RED);
	static void	debug(const std::string &msg, const std::string &color = RESET);
};

#endif /* LOGGER_HPP */