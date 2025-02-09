/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 15:29:54 by mgama             #+#    #+#             */
/*   Updated: 2025/02/09 11:00:35 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "daemon.hpp"

int	init_unix_socket()
{
	int sockfd;
	struct sockaddr_un addr;

	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Erreur lors de la création de la socket");
		return (WBS_ERR);
	}

	// Nettoyage de la structure d'adresse
	bzero(&addr, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, WBS_SOCKET_PATH, sizeof(addr.sun_path) - 1);

	if (bind(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
		perror("Erreur lors de la liaison de la socket à l'adresse");
		return (WBS_ERR);
	}

	printf("Socket Unix créée avec succès.\n");

	// Attente de connexion
	if (listen(sockfd, 5) == -1) {
		perror("Erreur lors de l'attente de connexion");
		return (WBS_ERR);
	}
	printf("En attente de connexion...\n");

	struct pollfd fds[1];
	fds[0].fd = sockfd;
	fds[0].events = POLLIN;
	while (1) {
		int ret = poll(fds, 1, -1);
		if (ret == -1) {
			perror("Erreur lors de l'appel à poll");
			return (WBS_ERR);
		}

		if (fds[0].revents & POLLIN) {
			int client_sockfd;
			struct sockaddr_un client_addr;
			socklen_t client_addr_len = sizeof(struct sockaddr_un);

			client_sockfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_addr_len);
			if (client_sockfd == -1) {
				perror("Erreur lors de l'acceptation de la connexion");
				return (WBS_ERR);
			}

			printf("Connexion acceptée.\n");

			// Réception des données
			char buffer[256];
			ssize_t bytes_received;

			bytes_received = recv(client_sockfd, buffer, sizeof(buffer), 0);
			if (bytes_received == -1) {
				perror("Erreur lors de la réception des données");
				return (WBS_ERR);
			} else {
				buffer[bytes_received] = '\0';
				printf("Données reçues : %s\n", buffer);
			}

			close(client_sockfd); // Fermeture de la connexion avec le client
		}
	}
	
	// Ton code peut continuer ici, par exemple en écoutant sur cette socket.
	unlink(WBS_SOCKET_PATH); // Supprime la socket Unix
	close(sockfd); // N'oublie pas de fermer la socket lorsque tu n'en as plus besoin

	return (WBS_SUCCESS);
}
