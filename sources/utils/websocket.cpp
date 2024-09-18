/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   websocket.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/18 15:36:50 by mgama             #+#    #+#             */
/*   Updated: 2024/09/18 15:54:01 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"
#include "utils.hpp"

void	decodeWebSocketFrame(const std::string& frame)
{
	if (frame.size() < 2) {
		return; // Trame trop courte
	}

	// Convertir la chaîne en vecteur d'octets
	std::vector<uint8_t> frameData(frame.begin(), frame.end());

	// Lire les 2 premiers octets pour obtenir les informations de la trame
	uint8_t byte1 = frameData[0];
	uint8_t byte2 = frameData[1];

	// Extraire les informations de la trame
	bool fin = (byte1 & 0x80) != 0;
	bool masked = (byte2 & 0x80) != 0;
	uint8_t opcode = byte1 & 0x0F;
	uint8_t payloadLen = byte2 & 0x7F;

	// Déterminer la longueur du payload
	size_t index = 2;
	if (payloadLen == 126) {
		payloadLen = (frameData[index] << 8) | frameData[index + 1];
        index += 2; // Incrémenter index après avoir lu la longueur étendue
	} else if (payloadLen == 127) {
		Logger::error("Payload length too large");
	}

	// Lire le masque si nécessaire
	std::vector<uint8_t> mask(4);
	if (masked) {
		for (int i = 0; i < 4; ++i) {
			mask[i] = frameData[index++];
		}
	}

	// Lire les données de payload
	if (index + payloadLen > frameData.size()) {
		Logger::error("Frame too short for specified payload length");
	}
	std::string payload(frameData.begin() + index, frameData.begin() + index + payloadLen);

	// Démasquer les données si nécessaire
	if (masked) {
		for (size_t i = 0; i < payload.size(); ++i) {
			payload[i] ^= mask[i % 4];
		}
	}

	std::cout << "===============" << std::endl;
	std::cout << "socket frame: ";
	std::cout << "FIN: " << fin << ", ";
    std::cout << "Opcode: " << static_cast<int>(opcode) << ", ";
    std::cout << "Payload Length: " << static_cast<int>(payloadLen) << std::endl;
    std::cout << "Payload Data: ";
    for (size_t i = 0; i < payload.size(); ++i) {
        std::cout << static_cast<char>(payload[i]);
    }
    std::cout << std::endl;
}

std::string	sendCloseFrame(uint16_t closeCode, const std::string& reason)
{
    // Construire la trame de fermeture
    std::vector<uint8_t> frame;
    
    // Ajouter le premier octet (FIN + opcode 0x8 pour close frame)
    frame.push_back(0x80 | 0x8);  // 0x80 signifie FIN = 1 et 0x8 est le opcode pour close frame
    
    // Ajouter le deuxième octet (longueur du payload)
    size_t payloadLength = 2 + reason.size(); // 2 octets pour le code de fermeture + taille de la raison
    if (payloadLength > 125) {
        Logger::error("Payload too long for a close frame");
    }
    frame.push_back(static_cast<uint8_t>(payloadLength));
    
    // Ajouter le code de fermeture (2 octets)
    frame.push_back(static_cast<uint8_t>(closeCode >> 8)); // Code de fermeture haut octet
    frame.push_back(static_cast<uint8_t>(closeCode & 0xFF)); // Code de fermeture bas octet
    
    // Ajouter la raison (si elle existe)
    if (!reason.empty()) {
        frame.insert(frame.end(), reason.begin(), reason.end());
    }
    
	return std::string(frame.begin(), frame.end());
}
