/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 16:34:49 by mgama             #+#    #+#             */
/*   Updated: 2024/01/05 12:26:09 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "readsocket/readsocket.hpp"

class Server
{
private:
	uint16_t			port;
	int					socket_fd;
	struct sockaddr_in	socket_addr;
	fd_set				_fd_set;

public:
	Server(int port);
	~Server(void);

	bool	exit;

	const int	init(void);
	const int	start(void);
	void		kill(void);

	const uint16_t	getPort(void) const;

	void	setPort(const uint16_t port);
};