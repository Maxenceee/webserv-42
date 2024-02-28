/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWorker.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 15:35:34 by mgama             #+#    #+#             */
/*   Updated: 2024/02/28 17:25:02 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIWORKER_HPP
#define CGIWORKER_HPP

#include "webserv.hpp"
#include "request/Request.hpp"

#ifndef CGIWORKER_BUFFER_SIZE
# define CGIWORKER_BUFFER_SIZE 2 << 15
#endif /* CGIWORKER_BUFFER_SIZE */

class CGIWorker
{
private:
	static t_mapss	init(const Request &req);
	static char		**getEnv(const t_mapss &env);

public:
	static std::string		run(const Request &req, const std::string &scriptpname);
};

#endif /* CGIWORKER_HPP */