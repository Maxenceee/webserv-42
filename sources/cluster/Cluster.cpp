/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/11 19:48:08 by mgama             #+#    #+#             */
/*   Updated: 2024/12/16 18:05:36 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"

bool Cluster::exit = true;
bool Cluster::kill = false;
ThreadPool Cluster::pool(0);

static void interruptHandler(int sig_int) {
	(void)sig_int;
	std::cout << "\b\b";
	Logger::print("Signal received: " + std::string(strsignal(sig_int)), B_GREEN);
	/**
	 * Si le signal reçu est SIGQUIT ou SIGTERM, on met la variable kill à true,
	 * pour forcer l'arrêt du serveur (notamment des workers).
	 */
	// if (sig_int == SIGQUIT || sig_int == SIGTERM)
	// {
	// 	Cluster::kill = true;
	// }
	/**
	 * Ou si l'utilisateur a déjà demandé l'arrêt du serveur, on force l'arrêt.
	 */
	// else if (Cluster::exit == true)
	// {
	// 	Logger::print("Forcing shutdown", B_GREEN);
	// }
	Cluster::kill = true;
	Cluster::exit = true;
}

Cluster::Cluster(void)
{
	this->exit = false;
	this->parser = new Parser(*this);
}

Cluster::~Cluster()
{
	Logger::print("Shutting down webserv", B_GREEN);
	if (this->parser)
		delete this->parser;
	for (wsb_v_servers_t::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
		delete *it;
	Logger::debug("Cluster destroyed");
}

void	Cluster::parse(const std::string &configPath)
{
	/**
	 * Initialisation de la librairie OpenSSL pour le chiffrement des données.
	 */
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();

	this->parser->parse(configPath);
	for (wsb_v_servers_t::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		(*it)->init();
		if (Logger::isDebug())
			std::cout << *(*it) << std::endl;
	}
	delete this->parser;
	this->parser = NULL;
}

void	Cluster::initConfigs(std::vector<ServerConfig *> &configs)
{
	for (std::vector<ServerConfig *>::iterator it = configs.begin(); it != configs.end(); it++)
	{
		this->addConfig(*it);
	}
}

Server	*Cluster::addConfig(ServerConfig *config)
{
	wsb_v_servers_t::iterator it;
	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		if ((*it)->getPort() == config->getPort() && (*it)->getAddress() == config->getAddress() && (*it)->hasSSL() == config->hasSSL())
		{
			(*it)->addConfig(config);
			return (*it);
		}
	}
	Server *server = new Server(this->_servers.size() + 1, config->getPort(), config->getAddress(), config->hasSSL());
	server->addConfig(config);
	this->_servers.push_back(server);
	return (server);
}

int		Cluster::start(void)
{
	std::vector<pollfd>				poll_fds;
	std::map<int, wbs_pollclient>	poll_clients;
	const int						timeout = WBS_POLL_TIMEOUT;
	wsb_v_servers_t::iterator		it;

	// On vérifie si les serveurs du cluster ont été initialisés avant de les démarrer
	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		if (!(*it)->isInit()) {
			Logger::error("could not start server: the server has not been properly initialized");
			return (WBS_SOCKET_ERR);
		}
	}

	// Gestion des signaux pour fermer proprement le serveur
	signal(SIGINT, interruptHandler);
	signal(SIGQUIT, interruptHandler);
	signal(SIGTERM, interruptHandler);
	// Ingore le signal SIGPIPE
	signal(SIGPIPE, SIG_IGN);

	/**
	 * Initialise la pool de threads pour la gestion des requêtes proxy (8 threads).
	 */
	const int availableThreads = sysconf(_SC_NPROCESSORS_ONLN);
	Logger::debug("Available threads on device: " + toString<int>(availableThreads));
	Cluster::initializePool(availableThreads);

	// Initialise le tableau des descripteurs à surveiller
	for (it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		int serverSocket = (*it)->getSocketFD();
		poll_fds.push_back((pollfd){serverSocket, POLLIN | POLLHUP | POLLERR, 0});
		poll_clients[serverSocket] = (wbs_pollclient){WBS_POLL_SERVER, *it};
	}

	for (wsb_v_servers_t::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
	{
		Server *server = *it;
		Logger::print("Listening on port " + toString<int>(server->getPort()), B_GREEN);

		/**
		 * La fonction listen() permet de marquer un socket comme étant en
		 * attente de connexions entrantes.
		 * 
		 * Elle prend en paramètres le descripteur de fichier du socket et la taille de la file
		 * d'attente.
		 */
		int error = listen(server->getSocketFD(), WBS_DEFAULT_MAX_WORKERS);
		if (error == -1)
		{
			Logger::perror("server error: an error occurred while listening");
			return (WBS_SOCKET_ERR);
		}
	}

	sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	int newClient;
	Client *client;
	// ProxyClient *proxy;

	do
	{
		std::vector<int>	to_remove;

		/**
		 * La fonction poll() est utilisée pour surveiller plusieurs descripteurs de
		 * fichiers en même temps, notamment des sockets, des fichiers, ou d'autres
		 * types de descripteurs, afin de déterminer s'ils sont prêts pour une lecture,
		 * une écriture (I/O) ou s'ils ont généré une exception.
		 * 
		 * La fonction prend un tableau de structures `pollfd` dans lequel il faut spécifier
		 * pour chaque élément, le descripteur de fichiers (pollfd::fd) et les événements à
		 * surveiller (pollfd::events).
		 * 
		 * La fonction attend que l'un des événements spécifiés se produise pour l'un
		 * des descripteurs surveillés ou jusqu'à ce que le timeout expire. L'événement détecté
		 * est écrit dans pollfd::revents.
		 * 
		 * Dans ce cas, elle permet de s'assurer que le descripteur du socket est
		 * prêt pour la lecture.
		 */
		if (poll(poll_fds.data(), poll_fds.size(), timeout) == -1)
		{
			// Si le signal d'interruption est reçu, on continue
			if (errno == EINTR) {
				// Logger::debug("Signal received: exiting the server");
				continue;
			}
			Logger::perror("server error: an error occurred while poll'ing");
			return (WBS_SOCKET_ERR);
		}

		// Vérifier chaque connexion pour voir si elle est prête pour la lecture
		for (size_t i = 0; i < poll_fds.size(); ++i)
		{
			// Si le client a fermé la connexion ou si une erreur s'est produite
			if (poll_fds[i].revents & POLLHUP)
			{
				Logger::debug("Connection closed by the client (event POLLHUP)");
				to_remove.push_back(i); // Ajoute l'indice de l'élément à supprimer
			}
			else if (poll_fds[i].revents & POLLERR)
			{
				Logger::debug("Socket error (POLLERR detected)");
				to_remove.push_back(i);
			}
			/**
			 * On s'assure ensuite que l'événement détecté est bien celui attendu. On évite
			 * les faux positifs lorsque le timeout expire ou lorsqu'il y a une exception.
			 */
			else if (poll_fds[i].revents & POLLIN)
			{
				/**
				 * Le fait d'utiliser une structure générique `poll_clients` permet de gérer de manière
				 * simultanée plusieurs types de descripteurs (serveurs, clients, proxy, etc.).
				 * Le type permet d'identifier le descripteur afin de le caster correctement avant de l'utiliser.
				 */
				switch (poll_clients[poll_fds[i].fd].type)
				{
				case WBS_POLL_SERVER:
					/**
					 * La fonction accept() est utilisée pour accepter une connexion entrante d'un client.
					 * Elle prend en paramètres le descripteur de fichier du socket ainsi que le pointeur
					 * d'une structure `sockaddr` où seront écrites les informations sur le client (adresse IP, port, etc.).
					 * 
					 * La fonction retourne un nouveau descripteur de fichier vers le client.
					 */
					/**
					 * Étant donné que les serveurs sont ajoutés dans l'ordre au début du tableau des descripteurs
					 * à surveiller, on peut déduire l'indice du serveur à partir de l'indice du descripteur.
					 * D'où l'utilisation de l'indice `i` pour récupérer le serveur correspondant dans `this->_servers`
					 * au lieu de devoir faire un reinterpret_cast<Server *>(poll_clients[poll_fds[i].fd].data) comme
					 * pour les clients.
					 */
					newClient = accept(this->_servers[i]->getSocketFD(), (sockaddr *)&client_addr, &len);
					if (newClient == -1)
					{
						Logger::perror("server error: an error occurred while accept'ing the client");
						continue;
					}
					client_addr.sin_addr.s_addr = ntohl(client_addr.sin_addr.s_addr); // Corrige l'ordre des octets de l'adresse IP (endianness)
					client_addr.sin_port = ntohs(client_addr.sin_port); // Corrige l'ordre des octets du port (endianness)

					/**
					 * On encapsule le client dans un try/catch pour gérer les exceptions créées lors de la création du client.
					 */
					try
					{
						/**
						 * Création et ajout d'un nouveau client à la liste des clients.
						 */
						poll_clients[newClient] = (wbs_pollclient){WBS_POLL_CLIENT, new Client(this->_servers[i], newClient, client_addr)};
					}
					catch(const std::exception& e)
					{
						Logger::error("server error: " + std::string(e.what()));
						close(newClient);
						break;
					}
					/**
					 * On ajoute le nouveau client à la liste des descripteurs à surveiller.
					 */
					poll_fds.push_back((pollfd){newClient, POLLIN, 0});
					break;

				case WBS_POLL_CLIENT:
					if ((client = reinterpret_cast<Client *>(poll_clients[poll_fds[i].fd].data))->process() != WBS_POLL_CLIENT_OK) {
						to_remove.push_back(i); // Ajoute l'indice de l'élément à supprimer
					}
					break;

				// case WBS_POLL_PROXY:
				// 	if ((proxy = reinterpret_cast<ProxyClient *>(poll_clients[poll_fds[i].fd].data))->process() != WBS_POLL_CLIENT_OK) {
				// 		to_remove.push_back(i); // Ajoute l'indice de l'élément à supprimer
				// 	}
				// 	break;
				}
			}
			// Si le descripteur n'est pas prêt pour la lecture et que c'est un client, on
			// s'assure qu'il ne dépasse pas le timeout
			else if (poll_clients[poll_fds[i].fd].type == WBS_POLL_CLIENT)
			{
				if ((reinterpret_cast<Client *>(poll_clients[poll_fds[i].fd].data))->timeout())
					to_remove.push_back(i);
			}
		}

		// Nous supprimons les éléments à partir de la fin du vecteur pour éviter les décalages d'index
		for (std::vector<int>::reverse_iterator it = to_remove.rbegin(); it != to_remove.rend(); it++)
		{
			// Sauvegarde du descripteur de l'élément à supprimer
			newClient = poll_fds[*it].fd;
			poll_fds.erase(poll_fds.begin() + *it);
			client = reinterpret_cast<Client *>(poll_clients[newClient].data);
			poll_clients.erase(newClient);
			delete client;
		}
	} while (false == this->exit);

	// On ferme les sockets des clients et libère la mémoire
	for (std::map<int, wbs_pollclient>::iterator it = poll_clients.begin(); it != poll_clients.end(); it++)
	{
		if (it->second.type == WBS_POLL_CLIENT)
			delete reinterpret_cast<Client *>(it->second.data);
	}

	// On éteint les serveurs et libère la mémoire
	for (wsb_v_servers_t::iterator it = this->_servers.begin(); it != this->_servers.end(); it++)
		(*it)->kill();

	// On ferme la pool de threads
	Cluster::pool.kill(Cluster::kill);

	return (WBS_NOERR);
}
