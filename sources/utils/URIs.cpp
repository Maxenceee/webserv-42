/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URIs.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/17 19:11:27 by mgama             #+#    #+#             */
/*   Updated: 2024/02/23 19:52:22 by mgama            ###   ########.fr       */
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
