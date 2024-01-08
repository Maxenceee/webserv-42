/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 11:38:42 by mgama             #+#    #+#             */
/*   Updated: 2024/01/08 17:46:24 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <algorithm>
#include <cstddef>

std::vector<std::string>	split(const std::string &str, char c);
std::string					join(const std::vector<std::string> &list, const std::string &c);
std::string					&pop(std::string &str);
std::string					&trim(std::string &str, char c);
std::string					readKey(const std::string &line);
std::string					readValue(const std::string &line);
std::string					&to_upper(std::string &str);
std::string					&to_lower(std::string &str);
std::string					&capitalize(std::string &str);
std::string					getExtension(const std::string &filename);
std::string					getIPAddress(int addr);
void						replace(std::string &buffer, std::string searchValue, std::string replaceValue);

template <typename T>
bool	contains(const T &container, const std::string &test);

/* fs */

void	printFileInfo(const char *filename);
int		isFile(const std::string &path);
bool	isDirectory(const std::string &path);

#include "list.tpp"