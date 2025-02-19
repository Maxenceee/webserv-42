/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 11:38:42 by mgama             #+#    #+#             */
/*   Updated: 2025/01/05 13:44:20 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <cstddef>
#include <map>
#include <ctime>
#include <numeric>
#include <iterator>

#include "logger/Logger.hpp"

template <typename T>
std::string					toString(T val);
std::string					toString(bool val);

std::vector<std::string>	split(const std::string &str, char c);
std::vector<std::string>	parseQuotedAndSplit(const std::string &input);
std::vector<std::string>	tokenize(const std::string &input);
std::string					join(std::vector<std::string> &list, const std::string &c);
std::string					&pop(std::string &str);
std::string					&shift(std::string &str);
std::string					&trim(std::string &str, char c = ' ');
std::string					readDirectiveKey(std::string &line);
std::string					readKey(const std::string &line);
std::string					readValue(const std::string &line);
std::string					to_upper(const std::string &str);
std::string					&to_upper(std::string &str);
std::string					&to_lower(std::string &str);
std::string					&capitalize(std::string &str);
std::string					getExtension(const std::string &filename);
std::string					&replace(std::string &buffer, const std::string &searchValue, const std::string &replaceValue);
std::string					&replaceAll(std::string &buffer, char searchValue, char replaceValue);

bool						isNumber(const std::string &str);

struct cropoutput {
	const std::string &value;
	cropoutput(const std::string &val) : value(val) {}
};
std::ostream&				operator<<(std::ostream& os, struct cropoutput value);
std::string					cropoutputs(const std::string &input);

std::string					getIPAddress(int addr);
uint32_t					setIPAddress(const std::string &addr);
bool						isIPAddressFormat(const std::string &addr);
bool						isIPAddress(const std::string &addr);

size_t						parseSize(const std::string &size);
std::string					getSize(int size);

time_t						parseTime(const std::string &timeStr);
std::string					getTime(time_t time);

bool						isDigit(const std::string &str);

/* list */

template <typename T>
bool	contains(const T &container, const std::string &test);

template <typename T>
T	&pop(T &container);

template <typename T>
T	&shift(T &container);

template <typename T>
std::string	last(T &container);

template <typename T>
std::string	toStringl(T &container, std::string separator = " ");

/* fs */

void	printFileInfo(const char *filename);
int		isFile(const std::string &path);
bool	isDirectory(const std::string &path);
void	listFilesInDirectory(const std::string &path, std::map<std::string, std::string> &fileMap, bool recursive = true);
void	listDirContent(const std::string dirpath);
std::string	getLastModifiedDate(const std::string filepath);

int	createFile(const std::string &path, const std::string &content);
int	appendFile(const std::string &path, const std::string &content);
int	deleteFile(const std::string &path);

std::string		resolve(std::string root, std::string path);

/* URIs */

std::string	decodeURIComponent(std::string encoded);

struct wbs_url {
	/**
	 * Protocol part of the URL.
	 */
	std::string	protocol;
	/**
	 * Host part of the URL.
	 */
	std::string	host;
	/**
	 * Port part of the URL if specified in thr URL.
	 */
	std::string	port_s;
	/**
	 * Port part of the URL.
	 */
	uint16_t	port;
	/**
	 * Path part of the URL.
	 */
	std::string	path;
	/**
	 * Query part of the URL.
	 */
	std::string	query;
	/**
	 * Fragment part of the URL.
	 */
	std::string	fragment;
};

wbs_url newURL(const std::string &url);

/* time */

double	getTimestamp();

#include "string.tpp"
#include "list.tpp"

#endif /* UTILS_HPP */