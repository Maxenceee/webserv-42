/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/23 20:48:53 by mgama             #+#    #+#             */
/*   Updated: 2024/02/23 20:56:46 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

class Logger
{
private:
	const bool	_debug;

public:
	Logger(const bool debug = false);

	static void	print(const std::string &msg, const bool error);
};
