/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   base64.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 14:57:50 by mgama             #+#    #+#             */
/*   Updated: 2024/09/18 15:36:41 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BASE64_HPP
#define BASE64_HPP

#include <iostream>
#include <string>
#include <vector>

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::vector<uint8_t> base64_decode(std::string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	uint8_t char_array_4[4], char_array_3[3];
	std::vector<uint8_t> ret;

	while (in_len-- && ( encoded_string[in_] != '=')) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
	}

	return ret;
}

std::string base64_encode(const std::vector<uint8_t>& input)
{
	std::string encoded;
	encoded.reserve(((input.size() / 3) + (input.size() % 3 > 0)) * 4);

	std::uint32_t temp = {0};
	std::vector<uint8_t>::const_iterator it = input.begin();

	for(std::size_t i = 0; i < input.size() / 3; ++i)
	{
		temp  = (*it++) << 16;
		temp += (*it++) << 8;
		temp += (*it++);
		encoded.append(1, base64_chars[(temp & 0x00FC0000) >> 18]);
		encoded.append(1, base64_chars[(temp & 0x0003F000) >> 12]);
		encoded.append(1, base64_chars[(temp & 0x00000FC0) >> 6 ]);
		encoded.append(1, base64_chars[(temp & 0x0000003F)      ]);
	}

	switch(input.size() % 3)
	{
	case 1:
		temp = (*it++) << 16;
		encoded.append(1, base64_chars[(temp & 0x00FC0000) >> 18]);
		encoded.append(1, base64_chars[(temp & 0x0003F000) >> 12]);
		encoded.append(2, '=');
		break;
	case 2:
		temp  = (*it++) << 16;
		temp += (*it++) << 8;
		encoded.append(1, base64_chars[(temp & 0x00FC0000) >> 18]);
		encoded.append(1, base64_chars[(temp & 0x0003F000) >> 12]);
		encoded.append(1, base64_chars[(temp & 0x00000FC0) >> 6 ]);
		encoded.append(1, '=');
		break;
	}

	return encoded;
}

std::string calculateSecWebSocketAccept(const std::string& key) {
    const std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string concatenated = key + guid;

    // Calculate SHA-1 hash
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(concatenated.c_str()), concatenated.size(), hash);

    // Encode hash to Base64
    std::vector<uint8_t> hashVec(hash, hash + sizeof(hash));
    return base64_encode(hashVec);
}

#endif /* BASE64_HPP */