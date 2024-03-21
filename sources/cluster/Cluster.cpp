/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:48:08 by mgama             #+#    #+#             */
/*   Updated: 2024/03/21 16:30:55 by mgama            ###   ########.fr       */
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
		if (Logger::_debug)
			std::cout << *(*it) << std::endl;
	}
}

Cluster::~Cluster()
{
	Logger::print("Shutting down webserv", B_GREEN);
	for (v_servers::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
		delete *it;
	delete this->parser;
}

void	Cluster::initConfigs(std::vector<ServerConfig *> configs)
{
	for (std::vector<ServerConfig *>::iterator it = configs.begin(); it != configs.end(); it++)
	{
		this->addConfig(*it);
	}
}

Server	*Cluster::addConfig(ServerConfig *config)
{
	v_servers::iterator it;
	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		if ((*it)->getPort() == config->getPort() && (*it)->getAddress() == config->getAddress())
		{
			(*it)->addConfig(config);
			return (*it);
		}
	}
	Server *server = new Server(this->_servers.size() + 1, config->getPort(), config->getAddress());
	server->addConfig(config);
	this->_servers.push_back(server);
	return (server);
}

const int Cluster::start(void)
{
	pollfd	poll_fds[this->_servers.size()];
	const int			timeout = 100;
	v_servers::iterator it;

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
		Logger::print("Listening on port " + toString<int>(server->getPort()), B_GREEN);

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
			Logger::error("server error: an error occurred while listening");
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
		 * pour chaque élément, le descripteur de fichiers (pollfd::fd) et l'événements à
		 * surveiller (pollfd::events).
		 * 
		 * La fonction attend que l'un des événements spécifiés se produise pour l'un
		 * des descripteurs surveillés ou jusqu'à ce que le timeout expire. L'évènement detecté
		 * est ecrit dans pollfd::revents.
		 * 
		 * Dans ce cas elle permet de s'assurer que le descripteur du socket est
		 * prêt pour la lecture.
		 */
		if (poll(poll_fds, this->_servers.size(), timeout) == -1)
		{
			// Si le signal d'interruption est reçu, on sort de la boucle
			if (errno == EINTR) {
				break ;
			}
			Logger::error("server error: an error occurred while poll'ing", RESET);
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
					Logger::error("server error: an error occurred while accept'ing the client", RESET);
					perror("accept");
					continue;
				}

				// if (fcntl(newClient, F_SETFL, O_NONBLOCK) == -1) {
				// 	perror("fcntl");
				// 	close(newClient);
				// 	continue;
				// }

				client_addr.sin_addr.s_addr = ntohl(client_addr.sin_addr.s_addr); // Converti l'adresse IP en notation décimale
				client_addr.sin_port = ntohs(client_addr.sin_port); // Converti le port en notation décimale
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

// #define MAX_CLIENTS 1024

// const int Cluster::start(void)
// {
// 	pollfd	poll_fds[this->_servers.size() + MAX_CLIENTS];
// 	const int			timeout = 100;
// 	v_servers::iterator it;

// 	// Gestion des signaux pour fermer proprement le serveur
// 	signal(SIGINT, interruptHandler);
// 	signal(SIGQUIT, interruptHandler);

// 	// On verifie si les serveurs du cluster ont été initialisé avant de le démarer
// 	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
// 	{
// 		if (!(*it)->isInit())
// 			throw Server::ServerNotInit();
// 	}

// 	// Initialise le tableau des descripteurs à surveiller
// 	int i = 0;
// 	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
// 	{
// 		int serverSocket = (*it)->getSocketFD();
// 		// if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1) {
//         //     perror("fcntl");
//         //     return (W_SOCKET_ERR);
//         // }
// 		// poll_fds.push_back((pollfd){serverSocket, POLLIN, 0});
// 		poll_fds[i++] = (pollfd){serverSocket, POLLIN | POLLPRI, 0};
// 	}

// 	for (v_servers::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
// 	{
// 		Server *server = *it;
// 		Logger::print("Listening on port " + toString<int>(server->getPort()), B_GREEN);

// 		/**
// 		 * La fonction listen() permet de marquer un socket comme étant un socket en
// 		 * attente de connexions entrantes.
// 		 * 
// 		 * Elle prend en paramètres le descripteur de fichiers du socket et la taille de la file
// 		 * d'attente.
// 		 */
// 		int error = listen(server->getSocketFD(), 1024);
// 		if (error == -1)
// 		{
// 			Logger::error("server error: an error occurred while listening");
// 			perror("listen");
// 			return (W_SOCKET_ERR);
// 		}
// 	}

// 	int useClient = this->_servers.size();

// 	sockaddr_in client_addr;
// 	socklen_t len = sizeof(client_addr);
// 	do
// 	{
// 		/**
// 		 * La fonction poll() est utilisée pour surveiller plusieurs descripteurs de
// 		 * fichiers en même temps, notamment des sockets, des fichiers, ou d'autres
// 		 * types de descripteurs, afin de déterminer s'ils sont prêts pour une lecture,
// 		 * une écriture (I/O) ou s'ils ont généré une exception.
// 		 * 
// 		 * La fonction prend un tableau de structures `pollfd` dans lequel il faut spécifier
// 		 * pour chaque élément, le descripteur de fichiers (pollfd::fd) et l'événements à
// 		 * surveiller (pollfd::events).
// 		 * 
// 		 * La fonction attend que l'un des événements spécifiés se produise pour l'un
// 		 * des descripteurs surveillés ou jusqu'à ce que le timeout expire. L'évènement detecté
// 		 * est ecrit dans pollfd::revents.
// 		 * 
// 		 * Dans ce cas elle permet de s'assurer que le descripteur du socket est
// 		 * prêt pour la lecture.
// 		 */
// 		// if (poll(poll_fds, this->_servers.size(), timeout) == -1)
// 		if (poll(poll_fds, useClient, timeout) == -1)
// 		{
// 			// Si le signal d'interruption est reçu, on sort de la boucle
// 			if (errno == EINTR) {
// 				break ;
// 			}
// 			Logger::error("server error: an error occurred while poll'ing", RESET);
// 			perror("poll");
// 			return (W_SOCKET_ERR);
// 		}

// 		// Vérifier chaque serveur pour les connexions entrantes
// 		for (size_t i = 0; i < this->_servers.size(); ++i)
// 		{
// 			/**
// 			 * On s'assure ensuite que l'évenement détécté est bien celui attendu. On évite
// 			 * les faux positifs lorsque timeout expire ou lorsqu'il y a une exception.
// 			 */
// 			if (poll_fds[i].revents & POLLIN)
// 			{
// 				/**
// 				 * La fonction accept() est utilisée pour accepter une connexion entrante d'un client.
// 				 * Elle prend en paramètres le descripteur de fichiers du socket ainsi que le pointeur
// 				 * d'une structure `sockaddr` ou seront écrite les informations sur le client (adresse IP, port, etc.).
// 				 * 
// 				 * La fonction retourne un nouveau descripteur de fichiers vers le client.
// 				 */
// 				int newClient = accept(this->_servers[i]->getSocketFD(), (sockaddr *)&client_addr, &len);
// 				if (newClient == -1)
// 				{
// 					Logger::error("server error: an error occurred while accept'ing the client", RESET);
// 					perror("accept");
// 					continue;
// 				}

// 				// if (fcntl(newClient, F_SETFL, O_NONBLOCK) == -1) {
// 				// 	perror("fcntl");
// 				// 	close(newClient);
// 				// 	continue;
// 				// }

// 				// client_addr.sin_addr.s_addr = ntohl(client_addr.sin_addr.s_addr); // Converti l'adresse IP en notation décimale
// 				// client_addr.sin_port = ntohs(client_addr.sin_port); // Converti le port en notation décimale
// 				// Traitement de la nouvelle connexion
// 				// this->_servers[i]->handleRequest(newClient, client_addr);
// 				for (int i = this->_servers.size(); i < MAX_CLIENTS; i++)
//                 {
//                     if (poll_fds[i].fd == 0)
//                     {

//                         poll_fds[i].fd = newClient;
//                         poll_fds[i].events = POLLIN | POLLPRI;
//                         useClient++;
//                         break;
//                     }
//                 }
// 				// poll_fds[this->_servers.size() + newClient] = (pollfd){newClient, POLLIN, 0};
// 			}
// 		}

// 		 for (int i = 1; i < MAX_CLIENTS; i++)
// 		{
// 			if (poll_fds[i].fd > 0 && poll_fds[i].revents & POLLIN)
// 			{
// 				char buf[RECV_SIZE];
// 				int bufSize = read(poll_fds[i].fd, buf, sizeof(buf));
// 				if (bufSize == -1)
// 				{
// 					poll_fds[i].fd = 0;
// 					poll_fds[i].events = 0;
// 					poll_fds[i].revents = 0;
// 					useClient--;
// 				}
// 				else if (bufSize == 0)
// 				{
// 					poll_fds[i].fd = 0;
// 					poll_fds[i].events = 0;
// 					poll_fds[i].revents = 0;
// 					useClient--;
// 				}
// 				else
// 				{
// 					buf[bufSize] = '\0';
// 					printf("From client: %s\n", buf);
// 				}
// 			}
// 		}
// 	} while (!this->exit);

// 	// Ferme les sockets des serveurs
// 	for (v_servers::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
// 		(*it)->kill();

// 	return (W_NOERR);
// }
