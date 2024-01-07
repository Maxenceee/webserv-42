/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strings.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 13:18:00 by mgama             #+#    #+#             */
/*   Updated: 2024/01/07 15:34:16 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "utils.hpp"

std::string		&pop(std::string& str)
{
	if (str.size())
		str.resize(str.size() - 1);
	return (str);
}

std::vector<std::string>		split(const std::string &str, char c)
{
	std::vector<std::string>	tokens;
	std::string					token;
	std::istringstream			tokenStream(str);

	while (std::getline(tokenStream, token, c))
		tokens.push_back(token);
	return (tokens);
}

std::string		&trim(std::string &str, char c)
{
	size_t	i;

	if (!str.size())
		return str;
	i = str.size();
	while (i && str[i - 1] == c)
		i--;
	str.resize(i);
	for (i = 0; str[i] == c; i++);
	str = str.substr(i, std::string::npos);
	return (str);
}

std::string		readKey(const std::string &line)
{
	std::string	ret;

	size_t	i = line.find_first_of(':');
	ret.append(line, 0 , i);
	capitalize(ret);
	return (trim(ret, ' '));
}

std::string		readValue(const std::string &line)
{
	size_t i;
	std::string	ret;

	i = line.find_first_of(':');
	i = line.find_first_not_of(' ', i + 1);
	if (i != std::string::npos)
		ret.append(line, i, std::string::npos);
	return (trim(ret, ' '));
}

std::string		&to_upper(std::string &str)
{
	std::transform(str.begin(), str.end(),str.begin(), ::toupper);
	return (str);
}

std::string		&to_lower(std::string &str)
{
	std::transform(str.begin(), str.end(),str.begin(), ::tolower);
	return (str);
}

std::string		&capitalize(std::string &str)
{
	size_t	i = 0;

	to_lower(str);
	str[i] = std::toupper(str[i]);
	while((i = str.find_first_of('-', i + 1)) != std::string::npos)
	{
		if (i + 1 < str.size())
		str[i + 1] = std::toupper(str[i + 1]);
	}
	return (str);
}

std::string		getExtension(const std::string &filename)
{
    size_t dotPosition = filename.find_last_of('.');
    if (dotPosition != std::string::npos && dotPosition < filename.length() - 1) {
        return (filename.substr(dotPosition + 1));
    }
    return ("");
}