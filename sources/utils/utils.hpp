/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 11:38:42 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 13:23:53 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <algorithm>
#include <cstddef>

std::vector<std::string>	split(const std::string &str, char c);
std::string					&pop(std::string &str);
std::string					&trim(std::string &str, char c);
std::string					readKey(const std::string &line);
std::string					readValue(const std::string &line);
std::string					&to_upper(std::string &str);
std::string					&to_lower(std::string &str);
std::string					&capitalize(std::string &str);

template <typename T>
bool	contains(const T &container, const std::string &test);

#include "list.tpp"