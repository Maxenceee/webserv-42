/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fs.cpp                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 19:22:57 by mgama             #+#    #+#             */
/*   Updated: 2024/12/20 14:45:25 by mgama            ###   ########.fr       */
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
		if (errno == ENOENT)
			Logger::debug("No such file or directory");
		else
			Logger::error(strerror(errno));
		return (false);
	}
	return S_ISDIR(pathStat.st_mode);
}

void listFilesInDirectory(const std::string &path, std::map<std::string, std::string> &fileMap, bool recursive)
{
	DIR				*dir;
	struct dirent	*ent;

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

std::string	getLastModifiedDate(const std::string filepath)
{
	struct stat fileStat;
	std::string res;
	
	if (stat(filepath.c_str(), &fileStat) == -1) {
		Logger::error("Error: could not get file information");
		return res;
	}

	struct tm* timeInfo = gmtime(&fileStat.st_mtime);
	char buffer[80];
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
	res = buffer;
	return (res);
}

int	createFile(const std::string &path, const std::string &content)
{
	std::ofstream file;

	file.open(path.c_str());
	if (!file.is_open())
	{
		Logger::error("Error: could not open file " + path);
		return (WBS_ERR);
	}
	file << content;
	file.close();
	return (WBS_SUCCESS);
}

int	appendFile(const std::string &path, const std::string &content)
{
	std::ofstream file;

	file.open(path.c_str(), std::ios::app);
	if (!file.is_open())
	{
		Logger::error("Error: could not open file " + path);
		return (WBS_ERR);
	}
	file << content;
	file.close();
	return (WBS_SUCCESS);
}

int	deleteFile(const std::string &path)
{
	if (remove(path.c_str()) != 0)
	{
		Logger::error("Error: could not delete file " + path);
		return (WBS_ERR);
	}
	return (WBS_SUCCESS);
}

std::string resolve(std::string root, std::string path)
{
	if (!root.empty() && root[root.size() - 1] == '/' && !path.empty() && path[0] == '/') {
		root.erase(root.size() - 1); // Supprimer le dernier '/' de root
	}

	// Vérifier si path est déjà présent à la fin de root
	if (!path.empty() && root.size() >= path.size() && root.compare(root.size() - path.size(), path.size(), path) == 0) {
		return root; // path est déjà présent à la fin de root, donc retourner root sans modification
	}

	return (root += path); // Ajouter path à la fin de root
}
