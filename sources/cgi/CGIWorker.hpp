/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWorker.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 15:35:34 by mgama             #+#    #+#             */
/*   Updated: 2024/02/06 12:40:10 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"

#ifndef CGIWORKER_BUFFER_SIZE
# define CGIWORKER_BUFFER_SIZE 2 << 15
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
