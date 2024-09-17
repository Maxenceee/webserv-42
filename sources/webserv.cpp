/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/10 11:51:04 by mgama             #+#    #+#             */
/*   Updated: 2024/09/17 14:28:54 by mgama            ###   ########.fr       */
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
	std::string fpath = argv[1];
	// Not the best way to do options parsing :/
	// But it works...
	if (std::string(argv[1]) == "--daemon")
	{
		fpath = "/etc/webserv/webserv.conf";
	}
	if (argc == 3) {
		if (std::string(argv[1]) == "-d" || std::string(argv[1]) == "--debug") {
			Logger::_debug = true;
			fpath = argv[2];
		} else if (std::string(argv[2]) == "-d" || std::string(argv[2]) == "--debug") {
			Logger::_debug = true;
			fpath = argv[1];
		}
	}
	Logger::init();
	Cluster	cluster;
	try
	{
		cluster.parse(fpath);
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
