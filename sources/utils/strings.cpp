/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strings.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/05 13:18:00 by mgama             #+#    #+#             */
/*   Updated: 2024/11/21 16:59:02 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "utils.hpp"
#include <numeric>
#include <iterator>

template <typename T>
std::string	toString(T val)
{
	std::stringstream stream;
	stream << val;
	return stream.str();
}

std::string	toString(bool val)
{
	return (val ? "true" : "false");
}

std::string		&pop(std::string &str)
{
	if (str.size())
		str.resize(str.size() - 1);
	return (str);
}

std::string		&shift(std::string &str)
{
	if (!str.empty())
		str.erase(str.begin());
	return str;
}

std::vector<std::string>	split(const std::string &str, char c)
{
	std::vector<std::string>	tokens;
	std::string					token;
	std::istringstream			tokenStream(str);

	while (std::getline(tokenStream, token, c))
		tokens.push_back(token);
	return (tokens);
}

std::vector<std::string>	parseQuotedAndSplit(const std::string &input)
{
	std::vector<std::string> result;
	std::string current;
	bool inQuotes = false;

	for (size_t i = 0; i < input.size(); ++i) {
		char c = input[i];

		if (c == '"') {
			inQuotes = !inQuotes; // Toggle the state
		} else if (c == ' ' && !inQuotes) {
			if (!current.empty()) {
				result.push_back(current);
				current.clear();
			}
		} else {
			current += c;
		}
	}

	if (!current.empty()) {
		result.push_back(current);
	}

	return result;
}

std::vector<std::string> tokenize(const std::string &line)
{
	std::vector<std::string> tokens;
	std::string token;
	bool inQuotes = false;

	for (size_t i = 0; i < line.length(); ++i) {
		char c = line[i];

		if (c == '"') {
			inQuotes = !inQuotes; // Toggle quote state
			token += c;
		} else if (c == '#' && !inQuotes) {
            // Ignore the rest of the line after a '#' if not inside quotes
            break;
        } else if ((isspace(c) || c == '\t') && !inQuotes) {
			if (!token.empty()) {
				tokens.push_back(token);
				token.clear();
			}
		} else if ((c == '{' || c == '}' || c == ';') && !inQuotes) {
			if (!token.empty()) {
				tokens.push_back(token);
				token.clear();
			}
			tokens.push_back(std::string(1, c)); // Add the '{', '}', or ';' as a token
		} else {
			token += c;
		}
	}

	if (!token.empty()) {
		tokens.push_back(token);
	}

	if (inQuotes) {
		throw std::runtime_error("Unterminated quote");
	}

	return tokens;
}

struct StringConcatenator {
	std::string separator;

	StringConcatenator(const std::string& sep) : separator(sep) {}

	std::string operator()(const std::string& acc, const std::string& element) const {
		return acc + separator + element;
	}
};

std::string		join(std::vector<std::string> &list, const std::string &c)
{
	if (list.empty()) {
		return ("");
	}
	std::vector<std::string>::iterator nextElement = list.begin();
	std::advance(nextElement, 1);
	return (std::accumulate(
		nextElement,
		list.end(),
		list[0],
		StringConcatenator(c)
	));
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

	if (line.empty())
		return ("");
	size_t	i = line.find_first_of(':');
	if (i == std::string::npos)
		return ("");
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

std::string		to_upper(const std::string &str)
{
	std::string tmp = str;
	std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
	return (tmp);
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

std::string		getIPAddress(int addr)
{
	// Similaire à inet_ntoa, renvoie une std::string et non un char *.
	std::string	res;

	res += toString((addr & 0xFF000000) >> 24);
	res += ".";
	res += toString((addr & 0xFF0000) >> 16);
	res += ".";
	res += toString((addr & 0xFF00) >> 8);
	res += ".";
	res += toString(addr & 0xFF);
	return (res);
}

uint32_t	setIPAddress(std::string addr)
{
	// Similaire à inet_addr, mais prend une std::string et non un char *, accepte localhost.
	if (addr == "localhost")
		return (INADDR_LOOPBACK);

	std::vector<std::string>	tokens = split(addr, '.');
	uint32_t					res = 0;
	if (tokens.size() != 4)
		return (0);
	res |= (std::atoi(tokens[0].c_str()) << 24);
	res |= (std::atoi(tokens[1].c_str()) << 16);
	res |= (std::atoi(tokens[2].c_str()) << 8);
	res |= std::atoi(tokens[3].c_str());
	return (res);
}

bool	isIPAddress(std::string addr)
{
	if (addr == "localhost")
		return (true);

	std::vector<std::string> tokens = split(addr, '.');
	if (tokens.size() != 4)
		return (false);
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (!isDigit(tokens[i]))
			return (false);
	}
	return (true);
}

std::string		&replace(std::string &buffer, std::string searchValue, std::string replaceValue)
{
	std::string	sv(searchValue);
	std::string	rv(replaceValue);
	std::size_t found_place = buffer.find(sv);

	while (found_place < UINT64_MAX)
	{
		buffer.erase(found_place, sv.length());
		buffer.insert(found_place, replaceValue);
		found_place = buffer.find(sv, found_place + 1);
	}
	return (buffer);
}

std::string &replaceAll(std::string &buffer, char searchValue, char replaceValue)
{
	size_t pos = 0;
	while ((pos = buffer.find(searchValue, pos)) != std::string::npos) {
		size_t end_pos = pos;
		// Find the end of the successive occurrences
		while (end_pos < buffer.size() && buffer[end_pos] == searchValue) {
			++end_pos;
		}
		// Replace all occurrences with a single one
		buffer.replace(pos, end_pos - pos, 1, replaceValue);
		pos += 1; // Move past the replaced character
	}
	return buffer;
}

size_t	parseSize(const std::string &size)
{
	size_t	ret = 0;
	size_t	i = 0;

	while (i < size.size() && size[i] >= '0' && size[i] <= '9')
	{
		ret = ret * 10 + size[i] - '0';
		i++;
	}
	if (i < size.size())
	{
		if (size[i] == 'b' || size[i] == 'B')
			ret *= 1;
		else if (size[i] == 'k' || size[i] == 'K')
			ret *= 1024;
		else if (size[i] == 'm' || size[i] == 'M')
			ret *= 1024 * 1024;
		else if (size[i] == 'g' || size[i] == 'G')
			ret *= 1024 * 1024 * 1024;
		else
			return (-1);
		if (i + 1 < size.size())
			return (-1);
	}
	return (ret);
}

std::string	getSize(int size)
{
	std::string	ret;
	if (size < 1024)
		ret = toString(size) + "B";
	else if (size < 1024 * 1024)
		ret = toString(size / 1024) + "KB";
	else if (size < 1024 * 1024 * 1024)
		ret = toString(size / (1024 * 1024)) + "MB";
	else
		ret = toString(size / (1024 * 1024 * 1024)) + "GB";
	return (ret);
}

time_t	parseTime(const std::string &timeStr)
{
	time_t	ret = 0;
	size_t	i = 0;

	while (i < timeStr.size() && timeStr[i] >= '0' && timeStr[i] <= '9')
	{
		ret = ret * 10 + timeStr[i] - '0';
		i++;
	}
	if (i < timeStr.size())
	{
		if (timeStr[i] == 's' || timeStr[i] == 'S')
			ret *= 1000;
		else if (timeStr[i] == 'm' || timeStr[i] == 'M')
			ret *= 60 * 1000;
		else if (timeStr[i] == 'h' || timeStr[i] == 'H')
			ret *= 60 * 60 * 1000;
		else if (timeStr[i] == 'd' || timeStr[i] == 'D')
			ret *= 24 * 60 * 60 * 1000;
		else
			return (-1);
		if (i + 1 < timeStr.size())
			return (-1);
	}
	return (ret);
}

std::string	getTime(time_t time)
{
	std::string	ret;
	if (time < 60 * 1000)
		ret = toString(time / 1000) + "s";
	else if (time < 60 * 60 * 1000)
		ret = toString(time / (60 * 1000)) + "m";
	else if (time < 24 * 60 * 60 * 1000)
		ret = toString(time / (60 * 60 * 1000)) + "h";
	else
		ret = toString(time / (24 * 60 * 60 * 1000)) + "d";
	return (ret);
}

bool isDigit(const std::string &str)
{
	return ::all_of(str.begin(), str.end(), ::isdigit);
}

std::ostream& operator<<(std::ostream& os, struct cropoutput value) {
	if (value.value.size() > 1000)
		os << value.value.substr(0, 1000) << "...";
	else
		os << value.value;
	return os;
}

std::string	cropoutputs(const std::string &input)
{
	if (input.size() > 1000)
		return (input.substr(0, 1000) + "...");
	return (input);
}

bool isNumber(const std::string &str)
{
	return ::all_of(str.begin(), str.end(), ::isdigit);
}