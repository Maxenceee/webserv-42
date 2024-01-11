/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:35:12 by mgama             #+#    #+#             */
/*   Updated: 2024/01/11 15:35:05 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "Server.hpp"
#include "response/Response.hpp"
#include "request/Request.hpp"

std::vector<std::string>	Server::methods = Server::initMethods();

Server::Server(int port): port(port)
{
	this->exit = false;
}

Server::~Server(void)
{
}

std::vector<std::string>	Server::initMethods()
{
	/**
	 * Seules les méthodes GET, POST et DELETE sont obligatoires, pour le moment
	 * toutes les méthodes sont présentes il faudra faire le tri par la suite en
	 * fonction de nos besoins.
	 * 
	 * La méthode HEAD est similaire à GET à ceci près qu'elle ne renvoie que l'en-tête
	 * de réponse.
	 * 
	 * Les méthodes PUT et PATCH sont similaires à POST, les différences étant la nature
	 * des modifications apportées. POST permet la création/ajout d'une ressource sur le serveur,
	 * PUT et PATCH permettent la modification de ladite ressource à la différence que PATCH
	 * a pour but de ne faire qu'une modification partielle.
	 * 
	 * La méthode TRACE est aussi assez simple, elle consiste à simplement retourner le contenu
	 * de la requête au client afin que celui-ci puisse évaluer la qualité de la connexion.
	 * 
	 * OPTION et CONNECT sont inutiles dans notre cas.
	 */
	std::vector<std::string>	methods;

	methods.push_back("GET");
	methods.push_back("HEAD");
	methods.push_back("POST");
	methods.push_back("PUT");
	methods.push_back("PATCH");
	methods.push_back("DELETE");
	methods.push_back("TRACE");
	methods.push_back("OPTIONS");
	methods.push_back("CONNECT");
	return methods;
}

const int	Server::init(void)
{
	int	option = 1;

	FD_ZERO(&this->_fd_set); // reset la liste de fildes
	std::cout << B_BLUE"Starting server" << RESET << std::endl;
	/**
	 * La fonction socket() permet de créer un point de terminaison (end-point) pour
	 * la communication réseau. 
	 * 
	 * La macro AF_INET permet de spécifier à la fonction quel type de connexion
	 * nous voulons, dans notre cas une connexion IPV4 (Internet Protocol Version 4) (pour se connecter à internet).
	 * Pour utiliser le protocole IPV6 il faut utiliser la macro AF_INET6. 
	 * 
	 * La macro SOCK_STREAM décrit le type de flux de données que nous voulons. Dans
	 * notre cas nous voulons utiliser la protocol TCP (Transmission Control Protocol)
	 * offrant un connexion fiable et sans perte de données. La fonction offre aussi
	 * la possibilité d'utiliser la protocole UDP (User Datagram Protocol) avec la
	 * macro SOCK_DGRAM privilégiant quant à lui la rapidité au détriment de la fiabilité. 
	 * 
	 * La fonction renvoie un descripteur de fichiers.
	 */
	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	FD_SET(this->socket_fd, &this->_fd_set); // permet d'ajouter à l'ensemble `fdset` (inutilisé pour le moment)
	if (this->socket_fd == -1)
	{
		std::cerr << W_PREFIX"error: Could not create socket" << std::endl;
		perror("socket");
		return (W_SOCKET_ERR);
	}
	/**
	 * La fonction setsockopt() permet de configurer notre socket créé précédemment.
	 * 
	 * Ici nous spécifions grâce à la macro SOL_SOCKET que le paramètre s'applique directement
	 * au socket.
	 * 
	 * SO_REUSEADDR permet de réutiliser une adresse locale immédiatement après que le socket
	 * associé ait été fermé. Cela peut être utile dans le cas où le programme se termine de manière
	 * inattendue et que l'adresse et le port qu'il utilisait sont toujours associés au socket
	 * pendant un certain temps.
	 */
	if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1) {
		perror("setsockopt");
		return (W_SOCKET_ERR);
	}

	/**
	 * Lorsque je le met plus rien de fonctionne, et lorsque j'accepte les requêtes recv()
	 * renvoie que le descripteur n'est pas prêt.
	 */
	// if (fcntl(this->socket_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) == -1) {
	// 	perror("fcntl");
	// 	return (W_SOCKET_ERR);
	// }

	memset(&this->socket_addr, 0, sizeof(this->socket_addr));
	this->socket_addr.sin_family = AF_INET;
	this->socket_addr.sin_port = htons(this->port);
	this->socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	/**
	 * La fonction bind permet d'attacher un socket à une adresse IP et un port
	 * de la machine hôte. Cela permet d'identifier de manière unique le socket
	 * sur le réseau. Cela signifie que le socket utilisera cette adresse
	 * pour communiquer sur le réseau.
	 * 
	 * Elle prend un paramètres le descripteur de fichiers du socket conserné et une structure
	 * de donné d'un type spéfique en fonction du type de connexion. Dans nôtre cas (INET)
	 * `sockaddr_in` ou `sockaddr_in6` en fonction de la version passé lors de la
	 * création du socket, dans notre cas nous utilisons IPV4 donc sockaddr_in.
	 * 
	 * - Le champs sin_family permet de spécifier le type connexion, il doit être identique
	 * à celui passé à la fonction socket() précédemment.
	 * - Le champs sin_port permet de spécifier le port auquel on souhaite attacher le socket. Le
	 * numéro du port doit être passé dans la fonction htons() en raison des différents ordres
	 * d'octet (endianness) entre le réseau et la machine hôte. Le réseau utilisant un Big-Endian
	 * cette fonction s'assure que la valeur passé soit convertie en conséquence. Cette conversion
	 * est éssentielle pour éviter les problèmes de compatibilités.
	 * - En enfin le champs sin_addr permet de spécifier l'adresse IP que nous voulons associer, 
	 * dans nôtre cas nous utilisons INADDR_ANY afin d'indiquer que le socket peut être associé à
	 * n'importe quelle adresse IP disponible sur la machine.
	 */
	int ret_conn = bind(this->socket_fd, (sockaddr *)&this->socket_addr, sizeof(this->socket_addr));
	if (ret_conn == -1)
	{
		std::cerr << W_PREFIX"error: Could not bind port" << std::endl;
		perror("bind");
		return (W_SOCKET_ERR);
	}
	this->setupRoutes();
	return (W_NOERR);
}

void	Server::setupRoutes(void)
{
	/**
	 * DEMO:
	 */
	/**
	 * En fonction de la configuration passé par l'utilisateur, nous créons des instances
	 * de la classe Router pour chaque `Location` spécifié.
	 */
	Router router1 = Router(*this, "/static");
	/**
	 * allowMethod() indique au router qu'elle méthode HTTP il peut servir. Si aucune méthode
	 * n'est spécifiée le router les accepte toutes.
	 */
	router1.allowMethod("GET");
	/**
	 * setRoot() indique au router son dossier `root`. Si root n'est pas spécifié le router hérite
	 * du serveur.
	 */
	router1.setRoot("./public/router_1");
	/**
	 * setAutoIndex() permet d'activer le listage des dossiers si aucun fichier d'index n'est trouvé
	 */
	router1.setAutoIndex(true);
	/**
	 * La méthode Server::use() permet d'ajouter un router au serveur.
	 */
	this->use(router1);
	// Router router2 = Router(*this, "/static/", true);
	// router2.setRoot("./public/router_2");
	// this->use(router2);
	/**
	 * Test des redirections
	 */
	Router router3 = Router(*this, "/redir");
	router3.allowMethod("GET");
	router3.setRedirection("/static", true);
	this->use(router3);
}

const int	Server::start(void)
{
	pollfd		fds;
	const int	timeout = 100;

	struct sockaddr_in	client_addr;
	socklen_t			len = sizeof(client_addr);

	std::cout << B_GREEN"Listening on port " << this->port << RESET << std::endl;
	/**
	 * La fonction listen() permet de marquer un socket comme étant un socket en
	 * attente de connexions entrantes.
	 * 
	 * Elle prend en paramètres le descripteur de fichiers du socket et la taille de la file
	 * d'attente.
	 */
	int	error = listen(this->socket_fd, 1024);
	if (error == -1)
	{
		std::cerr << W_PREFIX"server error: an error occured while listening" << std::endl;
		perror("listen");
		return (W_SOCKET_ERR);
	}

	/**
	 * Boucle principale du serveur.
	 */
	fds.fd = this->socket_fd;
	fds.events = POLLIN;
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
		if (poll(&fds, 1, timeout) == -1)
		{
			std::cerr << W_PREFIX"server error: an error occured while poll'ing" << std::endl;
			perror("poll");
			return (W_SOCKET_ERR);
		}
		/**
		 * On s'assure ensuite que l'évenement détécté est bien celui attendu. On évite
		 * les faux positifs lorsque timeout expire ou lorsqu'il y a une exception.
		 */
		if (fds.revents & POLLIN)
		{
			/**
			 * La fonction accept() est utilisée pour accepter une connexion entrante d'un client.
			 * Elle prend en paramètres le descripteur de fichiers du socket ainsi que le pointeur
			 * d'une structure `sockaddr` ou seront écrite les informations sur le client (adresse IP, port, etc.).
			 * 
			 * La fonction retourne un nouveau descripteur de fichiers vers le client.
			 */
			int newClient = accept(this->socket_fd, (sockaddr *)&client_addr, &len);
			if (newClient == -1)
			{
				perror("accept");
				continue;
			}

			/**
			 * Ça casse aussi tout.
			 */
			// if (fcntl(newClient, F_SETFL, O_NONBLOCK) == -1) {
			// 	perror("fcntl");
			// 	close(newClient);
			// 	continue;
			// }

			client_addr.sin_addr.s_addr = ntohl(client_addr.sin_addr.s_addr);
			client_addr.sin_port = ntohs(client_addr.sin_port);
			this->handleRequest(newClient, client_addr);
		}
	} while (!this->exit);
	close(this->socket_fd);
	return (W_NOERR);
}

void	Server::kill(void)
{
	this->exit = true;
	close(this->socket_fd);
}

void	Server::use(const Router &router)
{
	this->_routes.push_back(router);
}

const uint16_t	Server::getPort(void) const
{
	return (this->port);
}

void	Server::setPort(const uint16_t port)
{
	this->port = port;
}

// int		Server::setStaticDir(const std::string &path)
// {
// 	// listFilesInDirectory(path, this->static_dir);
// 	for (t_mapss::iterator it = this->static_dir.begin(); it != this->static_dir.end(); it++)
// 		std::cout << it->first << " -> " << it->second << std::endl;
// 	return (W_NOERR);
// }

// const t_mapss		Server::getStaticDir(void) const
// {
// 	return (this->static_dir);
// }

const std::vector<std::string>	Server::getMethods(void) const
{
	return (this->methods);
}

const std::string				Server::getRoot(void) const
{
	return (this->_root);
}

void	Server::setErrorPage(const int code, const std::string path)
{
	if (this->_error_page.count(code)) {
		std::cout << B_YELLOW"server info: overriding previous error page for " << code << RESET << std::endl;
	}
	this->_error_page[code] = path;
}

void	Server::handleRequest(const int client, sockaddr_in clientAddr)
{
	char buffer[RECV_SIZE] = {0};

	/**
	 * La fonction recv() sert à lire le contenu d'un descripteur de fichiers, ici
	 * le descripteur du client. À la difference de read, la fonction recv est
	 * spécifiquement conçue pour la lecture à partir de socket. Elle offre une meilleure
	 * gestion de la lecture dans un contexte de travaille en réseau.
	 */
	int valread = recv(client, buffer, sizeof(buffer), 0);
	if (valread == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			std::cerr << "Client not ready to read" << std::endl;
			return ;
		} else {
			perror("recv");
		}
		return ;
	} else if (valread == 0) {
		std::cerr << "Connection closed by the client" << std::endl;
		return ;
	}
	/**
	 * Pour chaque requête entrante, on créer une instance des classes Request et Response
	 * qui se chargent de l'interprétation des données de la requête et de la génération
	 * de la réponse.
	 */
	Request	request = Request(*this, std::string(buffer), client, clientAddr);
	std::cout << request << std::endl;
	Response response = Response(*this, request.getClientSocket(), request);
	this->handleRoutes(request, response);
	std::cout << response << std::endl;
	printf(B_YELLOW"------------------Client closed-------------------%s\n\n", RESET);
	close(client);
}

void	Server::handleRoutes(Request &req, Response &res)
{
	for (std::vector<Router>::iterator it = this->_routes.begin(); it != this->_routes.end(); it++) {
		it->route(req, res);
		if (!res.canSend())
			break;
	}
	/**
	 * Dans le cas où la route demandée n'a pu être géré par aucun des routers du serveur,
	 * on renvoie la réponse par defaut. (Error 404, Not Found)
	 */
	if (res.canSend())
	{
		res.sendNotFound().end();
	}
}