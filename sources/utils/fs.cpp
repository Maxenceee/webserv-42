/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fs.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 19:22:57 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 19:25:38 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "utils.hpp"

void printFileInfo(const char *filename)
{
    struct stat fileStat;

    if (stat(filename, &fileStat) == 0) {
        std::cout << "Nom: " << filename << std::endl;
        std::cout << "Taille: " << fileStat.st_size << " octets" << std::endl;
        std::cout << "Dernière modification: " << ctime(&fileStat.st_mtime);
        std::cout << "----------------------------------------\n";
    } else {
        perror("stat");
    }
}

int		isFile(const std::string &path)
{
	struct stat s;

	if (stat(path.c_str(), &s) == 0 )
	{
		if (s.st_mode & S_IFDIR)
			return (0);
		else if (s.st_mode & S_IFREG)
			return (1);
		else
			return (0);
	}
	return (0);
}

bool	isDirectory(const std::string &path)
{
    struct stat pathStat;

    if (stat(path.c_str(), &pathStat) != 0) {
        perror("stat");
        return (false);
    }
    return S_ISDIR(pathStat.st_mode);
}
