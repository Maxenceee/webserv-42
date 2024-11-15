/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   websocket.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/15 15:29:39 by mgama             #+#    #+#             */
/*   Updated: 2024/11/15 15:32:50 by mgama            ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP

#include "webserv.hpp"

std::string	decodeWebSocketFrame(const std::string& frame);
std::string	sendCloseFrame(uint16_t closeCode = 1000, const std::string& reason = "");
std::string	sendFrame(const std::string& message);

#endif /* WEBSOCKET_HPP */