/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fs.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 19:22:57 by mgama             #+#    #+#             */
/*   Updated: 2024/02/23 23:36:06 by mgama            ###   ########.fr       */
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

bool	isDirectory(const std::string &path, const bool throwError)
{
	struct stat pathStat;

	if (stat(path.c_str(), &pathStat) != 0) {
		if (throwError)
			perror("stat");
		return (false);
	}
	return S_ISDIR(pathStat.st_mode);
}

void listFilesInDirectory(const std::string &path, std::map<std::string, std::string> &fileMap, bool recursive)
{
	DIR				*dir;
	struct dirent	*ent;

	// if (!isDirectory(path.c_str()))
	// {
	// 	throw std::invalid_argument(B_RED"server error: Invalid static dir: "+path+RESET);
	// }

	if ((dir = opendir(path.c_str())) != NULL)
	{
		while ((ent = readdir(dir)) != NULL)
		{
			if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
			{
				char fullPath[PATH_MAX];
				snprintf(fullPath, sizeof(fullPath), "%s/%s", path.c_str(), ent->d_name);

				if (isDirectory(fullPath) && recursive) {
					listFilesInDirectory(fullPath, fileMap, recursive);
				} else {
					fileMap[ent->d_name] = fullPath;
				}
			}
		}
		closedir(dir);
	} else {
		perror("opendir");
	}
}

void	listDirContent(const std::string dirpath)
{
	std::map<std::string, std::string>	active_dir_files; // temp

	listFilesInDirectory(dirpath, active_dir_files);
	for (std::map<std::string, std::string>::iterator it = active_dir_files.begin(); it != active_dir_files.end(); it++)
		std::cout << it->first << " -> " << it->second << std::endl;
}

int	createFile(const std::string &path, const std::string &content)
{
	std::ofstream file;

	file.open(path.c_str());
	if (!file.is_open())
	{
		std::cerr << "Error: could not open file " << path << std::endl;
		return (W_ERR);
	}
	file << content;
	file.close();
	return (W_NOERR);
}

int	appendFile(const std::string &path, const std::string &content)
{
	std::ofstream file;

	file.open(path.c_str(), std::ios::app);
	if (!file.is_open())
	{
		std::cerr << "Error: could not open file " << path << std::endl;
		return (W_ERR);
	}
	file << content;
	file.close();
	return (W_NOERR);
}

int	deleteFile(const std::string &path)
{
	if (remove(path.c_str()) != 0)
	{
		std::cerr << "Error: could not delete file " << path << std::endl;
		return (W_ERR);
	}
	return (W_NOERR);
}