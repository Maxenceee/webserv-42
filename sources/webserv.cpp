/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 15:26:08 by mgama             #+#    #+#             */
/*   Updated: 2024/01/06 13:39:37 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "webserv.hpp"

int	main(int argc, char const **argv)
{
	if (argc != 2)
	{
		std::cerr << W_PREFIX"Invalid usage" << std::endl;
		std::cout << W_PREFIX"usage: [configuration file]" << std::endl;
		return (EXIT_FAILURE);
	}
	Server	server(W_DEFAULT_PORT);
	if (server.init())
		return (EXIT_FAILURE);
	// server.setStaticDir("./public");
	if (server.start())
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}
