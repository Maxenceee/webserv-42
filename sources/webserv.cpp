/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 15:26:08 by mgama             #+#    #+#             */
/*   Updated: 2024/01/14 17:10:25 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "cluster/Cluster.hpp"

void	print_name(void)
{
	std::cout << "\n\n\n" << HEADER;
	std::cout << "╔═════════════════════════════════════════════════════════════════════════╗\n";
	std::cout << "║                                                                         ║\n";
	std::cout << "║                                                                         ║\n";
	std::cout << "║                                                                         ║\n";
	std::cout << "║        __          __       _        _____                              ║\n";
	std::cout << "║        \\ \\        / /      | |      / ____|                             ║\n";
	std::cout << "║         \\ \\  /\\  / /  ___  | |__   | (___     ___   _ __  __   __       ║\n";
	std::cout << "║          \\ \\/  \\/ /  / _ \\ | '_ \\   \\___ \\   / _ \\ | '__| \\ \\ / /       ║\n";
	std::cout << "║           \\  /\\  /  |  __/ | |_) |  ____) | |  __/ | |     \\ V /        ║\n";
	std::cout << "║            \\/  \\/    \\___| |_.__/  |_____/   \\___| |_|      \\_/         ║\n";
	std::cout << "║                                                                         ║\n";
	std::cout << "║                                                                         ║\n";
	std::cout << "║                                                                         ║\n";
	std::cout << "║                                                                         ║\n";
	std::cout << "╚═════════════════════════════════════════════════════════════════════════╝\n\n\n";
	// std::cout << " _    _        _      _____                    \n";
	// std::cout << "| |  | |      | |    /  ___|                   \n";
	// std::cout << "| |  | |  ___ | |__  \\ `--.   ___  _ __ __   __\n";
	// std::cout << "| |/\\| | / _ \\| '_ \\  `--. \\ / _ \\| '__|\\ \\ / /\n";
	// std::cout << "\\  /\\  /|  __/| |_) |/\\__/ /|  __/| |    \\ V / \n";
	// std::cout << " \\/  \\/  \\___||_.__/ \\____/  \\___||_|     \\_/  \n\n\n";
	std::cout << RESET << std::endl;
}

int	main(int argc, char const **argv)
{
	print_name();
	if (argc != 2)
	{
		std::cerr << W_PREFIX"Invalid usage" << std::endl;
		std::cout << W_PREFIX"usage: [configuration file]" << std::endl;
		return (EXIT_FAILURE);
	}
	// Cluster	cluster(argv[1]);
	Server	server(W_DEFAULT_PORT);
	if (server.init())
		return (EXIT_FAILURE);
	if (server.start())
		return (EXIT_FAILURE);
	return (EXIT_SUCCESS);
}
