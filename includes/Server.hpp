/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:34:49 by mgama             #+#    #+#             */
/*   Updated: 2023/12/31 18:25:16 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 

#include "readsocket.hpp"

class Server
{
private:
	uint16_t			port;
	int					socket_fd;
	struct sockaddr_in	socket_addr;

public:
	Server(int port);
	~Server(void);

	bool	exit;

	const int	init(void);
	const int	run(void);

	const uint16_t	getPort(void) const;

	void	setPort(const uint16_t port);
};
