/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/10 11:51:04 by mgama             #+#    #+#             */
/*   Updated: 2024/06/19 00:00:36 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "cluster/Cluster.hpp"
#include "parser/Parser.hpp"
#include "daemon/daemon.hpp"

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
	std::cout << RESET << std::endl;
}

int	main(int argc, char const **argv)
{
	print_name();
	if (argc > 3 || argc < 2)
	{
		Logger::error("Invalid usage", RESET);
		Logger::error("usage: [configuration file]", RESET);
		return (EXIT_FAILURE);
	}
	if (argc == 3) {
		if (std::string(argv[1]) == "-d" || std::string(argv[1]) == "--debug") {
			Logger::_debug = true;
			argv[1] = argv[2];
		} else if (std::string(argv[2]) == "-d" || std::string(argv[2]) == "--debug") {
			Logger::_debug = true;
		}
	}
	Logger::init();
	Cluster	cluster;
	try
	{
		cluster.parse(argv[1]);
		// Logger::info("Attempting to become daemon");
		// if (become_daemon(WBS_NO_CLOSE_FILES) < 0)
		// {
		// 	Logger::error("Failed to become daemon");
		// 	return (1);
		// }
		// Logger::info("Daemon created successfully");
		cluster.start();
	}
	catch(const std::exception& e)
	{
		Logger::error(e.what());
		return (1);
	}
	return (0);
}
