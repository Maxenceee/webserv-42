/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 11:38:42 by mgama             #+#    #+#             */
/*   Updated: 2024/01/17 19:13:29 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <cstddef>
#include <map>

template <typename T>
std::string					toString(T val);

std::vector<std::string>	split(const std::string &str, char c);
std::string					join(std::vector<std::string> &list, const std::string &c);
std::string					&pop(std::string &str);
std::string					&shift(std::string &str);
std::string					&trim(std::string &str, char c = ' ');
std::string					readKey(const std::string &line);
std::string					readValue(const std::string &line);
std::string					&to_upper(std::string &str);
std::string					&to_lower(std::string &str);
std::string					&capitalize(std::string &str);
std::string					getExtension(const std::string &filename);
std::string					getIPAddress(int addr);
void						replace(std::string &buffer, std::string searchValue, std::string replaceValue);

/* list */

template <typename T>
bool	contains(const T &container, const std::string &test);

template <typename T>
T	&pop(T &container);

template <typename T>
T	&shift(T &container);

/* fs */

void	printFileInfo(const char *filename);
int		isFile(const std::string &path);
bool	isDirectory(const std::string &path);
void	listFilesInDirectory(const std::string &path, std::map<std::string, std::string> &fileMap, bool recursive = true);
void	listDirContent(const std::string dirpath);

// URIs

std::string	decodeURIComponent(std::string encoded);

#include "list.tpp"