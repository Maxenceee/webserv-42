/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URIs.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/17 19:11:27 by mgama             #+#    #+#             */
/*   Updated: 2024/12/18 10:50:02 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

/**
 * Le mécanisme d'encodage en pourcentage (%-encoding) est utilisé pour représenter un
 * octet de données dans un composant d'URI lorsque le caractère correspondant est en
 * dehors de l'ensemble autorisé ou sert de délimiteur. Il consiste à représenter un octet
 * par trois caractères, débutant par le caractère pourcentage (%) suivi des deux chiffres
 * hexadécimaux représentant la valeur numérique de l'octet. Les lettres hexadécimales 
 * majuscules et minuscules sont équivalentes, mais il est recommandé d'utiliser des
 * majuscules pour l'encodage en pourcentage des URIs.
 * 
 * (https://www.rfc-editor.org/rfc/rfc3986.html#section-2.1)
 */
std::string	decodeURIComponent(std::string encoded)
{
	std::string decoded = encoded;

	int dynamicLength = decoded.size() - 2;

	if (decoded.size() < 3)
		return decoded;

	for (int i = 0; i < dynamicLength; i++) {
		std::string haystack = decoded.substr(i, 3);

		if (haystack.size() == 3 && haystack[0] == '%' &&
			isxdigit(haystack[1]) && isxdigit(haystack[2])) {
			haystack.replace(0, 1, "0x");
			std::stringstream ss;
			ss << std::hex << haystack;
			int charCode;
			ss >> charCode;
			decoded.replace(decoded.begin() + i, decoded.begin() + i + 3, 1, static_cast<char>(charCode));
		}

		dynamicLength = decoded.size() - 2;
	}

	return (decoded);
}

std::string extract_match(const std::string &url, const regmatch_t &match) {
	if (match.rm_so == -1) return "";
	return url.substr(match.rm_so, match.rm_eo - match.rm_so);
}

wbs_url newURL(const std::string &url) {
	const char *url_regex = 
		"^(([^:/?#]+):)?(//([^/?#]*))?"
		"([^?#]*)"
		"(\\?([^#]*))?"
		"(#(.*))?$";

	regex_t regex;
	if (regcomp(&regex, url_regex, REG_EXTENDED) != 0) {
		throw std::runtime_error("Failed to compile regex.");
	}

	regmatch_t matches[10];

	if (regexec(&regex, url.c_str(), 10, matches, 0) != 0) {
		regfree(&regex);
		throw std::invalid_argument("Invalid URL format: " + url);
	}

	wbs_url result;

	result.protocol = extract_match(url, matches[2]);
	result.host = extract_match(url, matches[4]);

	// Gestion de l'hôte (host:port)
	std::string authority = extract_match(url, matches[4]);
	std::string::size_type colon_pos = authority.find(':');
	if (colon_pos != std::string::npos) {
		result.host = authority.substr(0, colon_pos);
		result.port_s = authority.substr(colon_pos + 1);
		if (result.port_s.length() > 0 && isNumber(result.port_s)) {
			result.port = static_cast<uint16_t>(std::atoi(result.port_s.c_str()));
		} else {
			throw std::invalid_argument("Invalid port in URL: " + result.port_s);
		}
	} else {
		if (result.protocol == "http") {
			result.port = 80;
		} else if (result.protocol == "https") {
			result.port = 443;
		}
	}

	result.path = extract_match(url, matches[5]);
	result.query = extract_match(url, matches[7]);
	result.fragment = extract_match(url, matches[9]);

	if (result.path.empty()) {
		result.path = "/";
	}

	regfree(&regex);
	return result;
}