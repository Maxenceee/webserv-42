/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWorker.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 15:35:34 by mgama             #+#    #+#             */
/*   Updated: 2024/02/05 15:43:23 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include <sys/wait.h>

#ifndef CGIWORKER_BUFFER_SIZE
# define CGIWORKER_BUFFER_SIZE 32768
#endif /* CGIWORKER_BUFFER_SIZE */

class CGIWorker
{
private:
	std::string		_path;
	std::string		_body;

public:
	CGIWorker(const std::string path);
	~CGIWorker();

	void	run(void);
};
