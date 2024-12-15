/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:35:12 by mgama             #+#    #+#             */
/*   Updated: 2024/12/15 13:39:08 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

std::ostream	&operator<<(std::ostream &os, const Server &server)
{
	server.print(os);
	os << std::endl;
	return os;
}

std::vector<std::string>	Server::methods = Server::initMethods();

Server::Server(int id, uint16_t port, uint32_t address, bool ssl):
	_id(id),
	_port(port),
	_address(address),
	_ssl_enabled(ssl)
{
	this->_default = NULL;
	this->_init = false;
}

Server::~Server(void)
{
	for (std::vector<ServerConfig *>::iterator it = this->_configs.begin(); it != this->_configs.end(); it++)
		delete *it;
	Logger::debug("Server destroyed");
}

std::vector<std::string>	Server::initMethods()
{
	/**
	 * Note: Seules les méthodes GET, POST et DELETE sont obligatoires pour le rendu du projet WEBSERV.
	 * 
	 * La méthode HEAD est similaire à GET à ceci près qu'elle ne renvoie que l'en-tête
	 * de réponse.
	 * 
	 * Les méthodes PUT et PATCH sont similaires à POST, les différences étant la nature
	 * des modifications apportées. POST permet la création/ajout d'une ressource sur le serveur,
	 * PUT et PATCH permettent la modification de ladite ressource à la différence que PATCH
	 * a pour but de ne faire qu'une modification partielle. Cependant PUT est idempotent, c'est-à-dire
	 * que si la ressource existe déjà, elle sera remplacée, si elle n'existe pas elle sera
	 * créée. POST et PATCH ne sont pas idempotents, elles peuvent être utilisées pour modifier une ressource
	 * plusieurs fois et obtenir des résultats différents.
	 * 
	 * La méthode TRACE est aussi assez simple, elle consiste à simplement retourner le contenu
	 * de la requête au client afin que celui-ci puisse évaluer la qualité de la connexion.
	 * 
	 * OPTIONS et CONNECT sont inutiles dans notre cas.
	 */
	std::vector<std::string>	methods;

	methods.push_back("GET");			// (https://developer.mozilla.org/fr/docs/Web/HTTP/Methods/GET)
	methods.push_back("HEAD");			// (https://developer.mozilla.org/fr/docs/Web/HTTP/Methods/HEAD)
	/** POST
	 * Codes de réponse:
	 * - 200 si le fichier a été modifié
	 * - 201 si le fichier a été créé
	 */
	methods.push_back("POST");			// (https://developer.mozilla.org/fr/docs/Web/HTTP/Methods/POST)
	/** PUT
	 * Codes de réponse:
	 * - 201 si le fichier a été créé
	 * - 204 si le fichier a été modifié
	 */
	methods.push_back("PUT");			// (https://developer.mozilla.org/fr/docs/Web/HTTP/Methods/PUT)
	// methods.push_back("PATCH");		// (https://developer.mozilla.org/fr/docs/Web/HTTP/Methods/PATCH) (non implémenté)
	/** DELETE
	 * Codes de réponse:
	 * - 200 si le fichier a été supprimé et que le serveur a renvoyé un message de confirmation
	 * - 202 si le fichier peut être supprimé
	 * - 204 si le fichier a été supprimé
	 */
	methods.push_back("DELETE");		// (https://developer.mozilla.org/fr/docs/Web/HTTP/Methods/DELETE)
	methods.push_back("TRACE");			// (https://developer.mozilla.org/fr/docs/Web/HTTP/Methods/TRACE)
	// methods.push_back("OPTIONS");	// (https://developer.mozilla.org/fr/docs/Web/HTTP/Methods/OPTIONS) (non implémenté)
	// methods.push_back("CONNECT");	// (https://developer.mozilla.org/fr/docs/Web/HTTP/Methods/CONNECT) (non implémenté)
	return methods;
}

int	Server::init(void)
{
	// on verifie si le port du serveur a été configuré
	if (this->_port == 0) {
		Logger::error("server error: could not start server: port not set");
		return (WBS_ERR);
	}

	Logger::print("Initiating server " + toString<int>(this->_id), B_GREEN);
	/**
	 * La fonction socket() permet de créer un point de terminaison (end-point) pour
	 * la communication réseau. 
	 * 
	 * La macro AF_INET permet de spécifier à la fonction quel type de connexion
	 * nous voulons, dans notre cas une connexion IPV4 (Internet Protocol Version 4) (pour se connecter à internet).
	 * Pour utiliser le protocole IPV6, il faut utiliser la macro AF_INET6. 
	 * 
	 * La macro SOCK_STREAM décrit le type de flux de données que nous voulons. Dans
	 * notre cas, nous voulons utiliser le protocole TCP (Transmission Control Protocol)
	 * offrant une connexion fiable et sans perte de données. La fonction offre aussi
	 * la possibilité d'utiliser le protocole UDP (User Datagram Protocol) avec la
	 * macro SOCK_DGRAM, privilégiant quant à lui la rapidité au détriment de la fiabilité. 
	 * 
	 * La fonction renvoie un descripteur de fichiers.
	 */
	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socket_fd == -1)
	{
		Logger::perror("server error: Could not create socket");
		return (WBS_SOCKET_ERR);
	}

	/**
	 * Option vaux 1 pour activer l'option SO_REUSEADDR.
	 */
	int	option = 1;
	/**
	 * La fonction setsockopt() permet de configurer notre socket créé précédemment.
	 * 
	 * Ici, nous spécifions grâce à la macro SOL_SOCKET que le paramètre s'applique directement
	 * au socket.
	 * 
	 * SO_REUSEADDR permet de réutiliser une adresse locale immédiatement après que le socket
	 * associé ait été fermé. Cela peut être utile dans le cas où le programme se termine de manière
	 * inattendue et que l'adresse et le port qu'il utilisait sont toujours associés au socket
	 * pendant un certain temps.
	 */
	if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1) {
		Logger::perror("server error: Could not set socket options");
		return (WBS_SOCKET_ERR);
	}

	bzero(&this->socket_addr, sizeof(this->socket_addr));
	this->socket_addr.sin_family = AF_INET;
	this->socket_addr.sin_port = htons(this->_port);
	this->socket_addr.sin_addr.s_addr = htonl(this->_address);
	/**
	 * La fonction bind permet d'attacher un socket à une adresse IP et un port
	 * de la machine hôte. Cela permet d'identifier de manière unique le socket
	 * sur le réseau. Cela signifie que le socket utilisera cette adresse
	 * pour communiquer sur le réseau.
	 * 
	 * Elle prend en paramètre le descripteur de fichier du socket concerné et une structure
	 * de données d'un type spécifique en fonction du type de connexion. Dans notre cas (INET),
	 * `sockaddr_in` ou `sockaddr_in6` en fonction de la version passée lors de la
	 * création du socket, dans notre cas nous utilisons IPV4 donc sockaddr_in.
	 * 
	 * - Le champ sin_family permet de spécifier le type de connexion, il doit être identique
	 * à celui passé à la fonction socket() précédemment.
	 * - Le champ sin_port permet de spécifier le port auquel on souhaite attacher le socket. Le
	 * numéro du port doit être passé dans la fonction htons() en raison des différents ordres
	 * d'octets (endianness) entre le réseau et la machine hôte. Le réseau utilisant un Big-Endian,
	 * cette fonction s'assure que la valeur passée soit convertie en conséquence. Cette conversion
	 * est essentielle pour éviter les problèmes de compatibilité.
	 * - Enfin, le champ sin_addr permet de spécifier l'adresse IP que nous voulons associer,
	 * dans notre cas nous utilisons INADDR_ANY par défaut afin d'indiquer que le socket peut être associé à
	 * n'importe quelle adresse IP disponible sur la machine.
	 */
	int ret_conn = bind(this->socket_fd, (sockaddr *)&this->socket_addr, sizeof(this->socket_addr));
	if (ret_conn == -1)
	{
		if (errno == EADDRINUSE)
		{
			Logger::error("server error: bind error: Address already in use");
		}
		else
		{
			Logger::error("server error: bind error");
		}
		close(this->socket_fd);
		return (WBS_ERR);
	}
	
	Logger::debug("Setting up SSL context");
	for (std::vector<ServerConfig *>::iterator it = this->_configs.begin(); it != this->_configs.end(); it++) {
		if ((*it)->hasSSL()) {
			/**
			 * Si une erreur survient lors de la configuration du SSL, on retourne une erreur.
			 */
			if (!(*it)->setupSSL()) {
				return (WBS_ERR);
			}
		}
	}
	this->_init = true;
	return (WBS_NOERR);
}

void	Server::kill(void)
{
	close(this->socket_fd);
}

bool		Server::isInit(void) const
{
	return (this->_init);
}

int		Server::getSocketFD(void) const
{
	return (this->socket_fd);
}

void	Server::addConfig(ServerConfig *config)
{
	if (!this->_default) {
		this->_default = config;
	} else if (config->evalName("_") && !this->_default->evalName("_")) {
		this->_default = config;
	} else if (config->evalName("_") && this->_default->evalName("_")) {
		Logger::warning("server warning: multiple default configurations, only the first one will be used.");
	}
	config->setServer(this);
	config->used = true;
	this->_configs.push_back(config);
}

ServerConfig	*Server::getDefault(void) const
{
	return (this->_default);
}

uint32_t	Server::getAddress(void) const
{
	return (this->_address);
}

uint16_t	Server::getPort(void) const
{
	return (this->_port);
}

bool	Server::hasSSL(void) const
{
	return (this->_ssl_enabled);
}

ServerConfig	*Server::getConfig(const char *name) const
{
	return (this->getConfig(std::string(name)));
}

ServerConfig	*Server::getConfig(const std::string &name) const
{
	for (std::vector<ServerConfig *>::const_iterator it = this->_configs.begin(); it != this->_configs.end(); it++) {
		if ((*it)->evalName(name)) {
			return (*it);
		}
	}
	return (this->_default);
}

Router	*Server::eval(Request &request, Response &response) const
{	
	Logger::debug("Evaluating server config for name " + request.getHost() + ":" + toString<int>(request.getPort()));

	/**
	 * On cherche la configuration du serveur correspondant à l'hôte de la requête.
	 * Si aucun nom de serveur n'est spécifié ou s'il n'a pas de configuration définie, on utilise
	 * la configuration par défaut.
	 */
	Router	*router = this->getConfig(request.getHost())->getDefaultHandler();

	/**
	 * On retourne le résultat de l'évaluation du router, c'est-à-dire le plus approprié pour
	 * traiter la requête. Cette fonction regarde récursivement les sous-routers
	 * définis dans le router et retourne celui qui correspond le mieux à la requête.
	 */
	return (router->eval(request.getPath(), request.getMethod(), response));
}

void	Server::printResponse(const Request &req, const Response &res, const double response_duration)
{
	std::string response = req.getMethod() + " " + req.getRawPath() + " ";
	int status = res.getStatus();
	switch (status / 100)
	{
	case 1:
		response += GREY;
		break;
	case 2:
		response += GREEN;
		break;
	case 3:
		response += CYAN;
		break;
	case 4:
		response += YELLOW;
		break;
	case 5:
	default:
		response += RED;
		break;
	}
	response += toString<int>(status);
	response += RESET;

	// double response_duration = getTimestamp() - req.getRequestTime();
	std::stringstream ss;
	ss << std::fixed << std::setprecision(3) << response_duration;
	std::string duration_str = ss.str();
	response += " " + duration_str + " ms - ";

	if (res.hasBody())
		response += toString<int>(res.getBody().size());
	else
		response += "-";
	Logger::print(response);
}

void	Server::printProxyResponse(const std::string &method, const std::string &path, const double response_duration)
{
	std::string response = method + " " + path;

	std::stringstream ss;
	ss << std::fixed << std::setprecision(3) << response_duration;
	std::string duration_str = ss.str();
	response += " " + duration_str + " ms - ";
	Logger::print(response);
}

bool	Server::isValidMethod(const std::string &method)
{
	return (contains(Server::methods, method));
}

void	Server::print(std::ostream &os) const
{
	os << B_BLUE << "<--- Server " << this->_id << " --->" << RESET << "\n";
	os << B_CYAN << "Initiated: " << RESET << (this->_init ? "true" : "false") << "\n";
	os << B_CYAN << "Address: " << RESET << getIPAddress(this->_address) << "\n";
	os << B_CYAN << "Port: " << RESET << this->_port << "\n";
	os << B_CYAN << "SSL: " << RESET << (this->_ssl_enabled ? "enabled" : "disabled") << "\n";
	os << "\n";
	os << B_ORANGE << "Configurations: " << RESET << "\n";
	for (std::vector<ServerConfig *>::const_iterator it = this->_configs.begin(); it != this->_configs.end(); it++) {
		os << **it;
	}
}
