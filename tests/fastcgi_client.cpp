#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h> 

class Client {
private:
	int sockfd;
	struct sockaddr_in server_addr;

public:
	Client(const std::string& host, int port) {
		// Créer une socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1) {
			std::cerr << "Erreur lors de la création de la socket" << std::endl;
			exit(EXIT_FAILURE);
		}

		// Initialiser les informations sur le serveur
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port);
		server_addr.sin_addr.s_addr = inet_addr(host.c_str());

		// Établir la connexion avec le serveur
		if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
			std::cerr << "Erreur lors de la connexion au serveur FastCGI" << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	~Client() {
		// Fermer la connexion
		close(sockfd);
	}

	void sendRequest(const std::string& method, const std::string& path, const std::string& body) {
		// Construire la requête HTTP
		std::string request = method + " " + path + " HTTP/1.1\r\n";
		request += "Host: localhost\r\n";
		request += "Content-Length: " + std::to_string(body.length()) + "\r\n";
		request += "Content-Type: text/plain\r\n";
		// request += "Transfer-Encoding: chunked\r\n";
		request += "\r\n"; // Ligne vide pour séparer les en-têtes du corps
		request += body;
		// request += "8\r\n";
		// request += "ABCDEFGH\r\n";
		// request += "6\r\n";
		// request += "ABCDEF\r\n";
		// request += "a\r\n";
		// request += "ABCDEFGHIJ\r\n";
		// request += "0\r\n\r\n";

		// Envoyer la requête HTTP au serveur
		std::cout << "Request: " << request << std::endl;
		send(sockfd, request.c_str(), request.length(), 0);
	}


	std::string getResponse() {
		std::string response;
		char buffer[2 << 18];
		int bytesReceived;

		bytesReceived = read(sockfd, buffer, sizeof(buffer));
		response.append(buffer, bytesReceived);

		return response;
	}
};

int main(int argc, char **argv) {
	// Exemple d'utilisation
	Client client("127.0.0.1", 3001);
	// client.sendRequest("POST", "/oui", "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Demo</title></head><body><h1>Thanks for using Webserv</h1><a href=\"/info.html\"><p>see more</p></a></body></html>");
	client.sendRequest(argv[1], argv[2], argv[3]);
	std::string response = client.getResponse();
	std::cout << "Response from server: " << response << std::endl;

	return 0;
}
// POST /oui HTTP/1.1\r\nHost: localhost\r\nContent-Length: 8\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nreq.body