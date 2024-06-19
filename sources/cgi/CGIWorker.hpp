/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIWorker.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/05 15:35:34 by mgama             #+#    #+#             */
/*   Updated: 2024/04/18 13:16:50 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIWORKER_HPP
#define CGIWORKER_HPP

#include "webserv.hpp"
#include "request/Request.hpp"

#ifndef WBS_CGIWORKER_BUFFER_SIZE
# define WBS_CGIWORKER_BUFFER_SIZE 2 << 15
#endif /* WBS_CGIWORKER_BUFFER_SIZE */

class CGIWorker
{
private:
	static wbs_mapss_t	init(const Request &req, const std::string &scriptpath, wbs_mapss_t &params, const std::string &body);
	static char			**getEnv(const wbs_mapss_t &env);

public:
	static std::string	run(const Request &req, const std::string &scriptpath, wbs_mapss_t &params, const std::string &scriptpname, const std::string &body);
};

#endif /* CGIWORKER_HPP */