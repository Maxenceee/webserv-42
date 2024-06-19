/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   daemon.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 15:39:02 by mgama             #+#    #+#             */
/*   Updated: 2024/06/18 22:03:38 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "daemon.hpp"

int // retourne 0 en cas de succès, -1 en cas d'erreur
become_daemon(int flags)
{
	int maxfd, fd;

	/**
	 * Le premier fork va changer notre pid
	 * mais le sid et le pgid seront ceux
	 * du processus appelant.
	 */
	switch(fork())
	{
		case -1:
			perror("fork");
			return -1;
		case 0: break;                  // l'enfant continue
		default: _exit(EXIT_SUCCESS);   // le parent se termine
	}

	/**
	 * Exécutez le processus dans une nouvelle session sans terminal
	 * de contrôle. L'ID de groupe de processus sera l'ID du processus
	 * et donc, le processus sera le chef de groupe de processus.
	 * Après cet appel, le processus sera dans une nouvelle session,
	 * et il sera le chef de groupe de processus dans un nouveau
	 * groupe de processus.
	 */
	if(setsid() == -1) {               // devenir le leader de la nouvelle session
		perror("setsid");
		return -1;
	}
	/**
	 * Nous allons forker à nouveau, également connu sous le nom de
	 * double fork. Ce deuxième fork rendra notre processus orphelin
	 * car le parent se terminera. Lorsque le processus parent se termine,
	 * le processus enfant sera adopté par le processus init avec l'ID 1.
	 * Le résultat de ce deuxième fork est un processus dont le parent
	 * est le processus init avec un ID de 1. Le processus sera dans sa
	 * propre session et groupe de processus et n'aura aucun terminal
	 * de contrôle. De plus, le processus ne sera pas le chef de groupe
	 * de processus et donc, ne pourra pas avoir de terminal de contrôle
	 * s'il y en avait un.
	 */
	switch(fork())
	{
		case -1:
			perror("fork");
			return -1;
		case 0: break;                  // l'enfant sort continue
		default: _exit(EXIT_SUCCESS);   // le processus parent se terminera
	}

	if(!(flags & WBS_NO_UMASK0))
	    umask(0);                       // effacer le masque de création de fichiers

	if(!(flags & WBS_NO_CHDIR))
	    chdir("/");                     // changer vers le répertoire racine

	if(!(flags & WBS_NO_CLOSE_FILES))   // fermer tous les fichiers ouverts
	{
	    maxfd = sysconf(_SC_OPEN_MAX);  // permet de déterminer le nombre maximal de descripteurs disponibles
    if (maxfd == -1)
        maxfd = WBS_MAX_CLOSE;         // en cas d'erreur on utilise la valeur par défaut
    for (fd = 0; fd < maxfd; fd++) {
        if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO) { 
            // Ne pas fermer stdin, stdout et stderr
            close(fd);
        }
    }
	}

	if(!(flags & WBS_NO_REOPEN_STD_FDS))
	{
		/**
		 * On ferme stdin, puis on redirige stdout et stderr
		 * vers /dev/null
		 */
		close(STDIN_FILENO);

		fd = open("/dev/null", O_RDWR);
		if(fd != STDIN_FILENO) {
			 perror("open /dev/null");
			return -1;
		}
		if(dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
			perror("dup2 stdout");
			return -2;
		}
		if(dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
			perror("dup2 stderr");	
			return -3;
		}
	}

	Logger::debug("Daemon initialization complete");
	return 0;
}
