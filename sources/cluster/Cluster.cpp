/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:48:08 by mgama             #+#    #+#             */
/*   Updated: 2024/02/23 23:03:44 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"

bool Cluster::exit = true;

static void interruptHandler(int sig_int) {
	(void)sig_int;
	std::cout << "\b\b \b\b";
	Cluster::exit = false;
}

Cluster::Cluster(const char *configPath)
{
	this->exit = false;
	this->parser = new Parser(*this);
	this->parser->parse(configPath);
	for (v_servers::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		(*it)->init();
		std::cout << *(*it) << std::endl;
	}
}

Cluster::~Cluster()
{
	std::cout << B_GREEN << "Shutting down webserv" << RESET << std::endl;
	for (v_servers::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
		delete *it;
	delete this->parser;
}

Server	*Cluster::newServer(void)
{
	Server	*server = new Server(this->_servers.size() + 1);
	this->_servers.push_back(server);
	return (server);
}

const int Cluster::start(void)
{
	v_servers::iterator it;
	// std::vector<pollfd>	poll_fds;
	pollfd	poll_fds[this->_servers.size()];
	const int			timeout = 100;

	// Gestion des signaux pour fermer proprement le serveur
	signal(SIGINT, interruptHandler);
	signal(SIGQUIT, interruptHandler);

	// On verifie si les serveurs du cluster ont été initialisé avant de le démarer
	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		if (!(*it)->isInit())
			throw Server::ServerNotInit();
	}

	// Initialise le tableau des descripteurs à surveiller
	int i = 0;
	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		int serverSocket = (*it)->getSocketFD();
		// if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1) {
        //     perror("fcntl");
        //     return (W_SOCKET_ERR);
        // }
		// poll_fds.push_back((pollfd){serverSocket, POLLIN, 0});
		poll_fds[i++] = (pollfd){serverSocket, POLLIN, 0};
	}

	for (v_servers::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		Server *server = *it;
		std::cout << B_GREEN"Listening on port " << server->getPort() << RESET << std::endl;

		/**
		 * La fonction listen() permet de marquer un socket comme étant un socket en
		 * attente de connexions entrantes.
		 * 
		 * Elle prend en paramètres le descripteur de fichiers du socket et la taille de la file
		 * d'attente.
		 */
		int error = listen(server->getSocketFD(), 1024);
		if (error == -1)
		{
			std::cerr << "server error: an error occurred while listening" << std::endl;
			perror("listen");
			return (W_SOCKET_ERR);
		}
	}

	sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	do
	{
		/**
		 * La fonction poll() est utilisée pour surveiller plusieurs descripteurs de
		 * fichiers en même temps, notamment des sockets, des fichiers, ou d'autres
		 * types de descripteurs, afin de déterminer s'ils sont prêts pour une lecture,
		 * une écriture (I/O) ou s'ils ont généré une exception.
		 * 
		 * La fonction prend un tableau de structures `pollfd` dans lequel il faut spécifier
		 * pour chaque élément, le descripteur de fichiers et l'événements à surveiller.
		 * 
		 * La fonction attend que l'un des événements spécifiés se produise pour l'un
		 * des descripteurs surveillés ou jusqu'à ce que le timeout expire.
		 * 
		 * Dans ce cas elle permet de s'assurer que le descripteur de fichiers du socket est
		 * prêt pour la lecture.
		 */
		// if (poll(&poll_fds[0], poll_fds.size(), timeout) == -1)
		if (poll(poll_fds, this->_servers.size(), timeout) == -1)
		{
			if (errno == EINTR) {
				break ;
			}
			std::cerr << "server error: an error occurred while poll'ing" << std::endl;
			perror("poll");
			return (W_SOCKET_ERR);
		}

		// Vérifier chaque serveur pour les connexions entrantes
		for (size_t i = 0; i < this->_servers.size(); ++i)
		{
			/**
			 * On s'assure ensuite que l'évenement détécté est bien celui attendu. On évite
			 * les faux positifs lorsque timeout expire ou lorsqu'il y a une exception.
			 */
			if (poll_fds[i].revents & POLLIN)
			{
				/**
				 * La fonction accept() est utilisée pour accepter une connexion entrante d'un client.
				 * Elle prend en paramètres le descripteur de fichiers du socket ainsi que le pointeur
				 * d'une structure `sockaddr` ou seront écrite les informations sur le client (adresse IP, port, etc.).
				 * 
				 * La fonction retourne un nouveau descripteur de fichiers vers le client.
				 */
				int newClient = accept(this->_servers[i]->getSocketFD(), (sockaddr *)&client_addr, &len);
				if (newClient == -1)
				{
					perror("accept");
					continue;
				}

				// if (fcntl(newClient, F_SETFL, O_NONBLOCK) == -1) {
				// 	perror("fcntl");
				// 	close(newClient);
				// 	continue;
				// }

				client_addr.sin_addr.s_addr = ntohl(client_addr.sin_addr.s_addr);
				client_addr.sin_port = ntohs(client_addr.sin_port);
				// Traitement de la nouvelle connexion
				this->_servers[i]->handleRequest(newClient, client_addr);
			}
		}
	} while (!this->exit);

	// Ferme les sockets des serveurs
	for (v_servers::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
		(*it)->kill();

	return (W_NOERR);
}
