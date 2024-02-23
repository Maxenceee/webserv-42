/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/10 11:51:04 by mgama             #+#    #+#             */
/*   Updated: 2024/02/23 23:01:27 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "cluster/Cluster.hpp"
#include "parser/Parser.hpp"

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
	if (argc != 2)
	{
		std::cerr << W_PREFIX"Invalid usage" << std::endl;
		std::cout << W_PREFIX"usage: [configuration file]" << RESET << std::endl;
		return (EXIT_FAILURE);
	}
	try
	{
		Cluster	cluster(argv[1]);
		cluster.start();
	}
	catch(const std::exception& e)
	{
		std::cerr << B_RED << W_PREFIX << RESET << e.what() << std::endl;
		return (1);
	}
	return (0);
}
