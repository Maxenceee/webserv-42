/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/10 11:51:04 by mgama             #+#    #+#             */
/*   Updated: 2024/12/22 13:21:42 by mgama            ###   ########.fr       */
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
	// Pas la meilleur façon de faire :/
	// Mais ça marche...
	if (std::string(argv[1]) == "--daemon")
	{
		fpath = "/etc/webserv/webserv.conf";
	}
	if (argc == 3) {
		if (std::string(argv[1]) == "-d" || std::string(argv[1]) == "--debug") {
			Logger::setDebug(true);
			fpath = argv[2];
		} else if (std::string(argv[2]) == "-d" || std::string(argv[2]) == "--debug") {
			Logger::setDebug(true);
			fpath = argv[1];
		}
	}

	/**
	 * Les signaux d'arrêt sont ignorés tant que le parsing et le démarrage du serveur
	 * n'ont pas été effectués.
	 */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	int status = 0;
	Logger::init();
	Cluster	cluster;
	try
	{
		cluster.parse(fpath);

		status = cluster.start();
	}
	catch(const std::exception &e)
	{
		Logger::error(e.what());
		return (1);
	}
	Logger::print("Webserv successfully stopped", B_GREEN);
	return (status);
}
