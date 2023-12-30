/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 15:26:17 by mgama             #+#    #+#             */
/*   Updated: 2023/12/30 17:02:41 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include "pcolors.hpp"

/* text */

#define W_PREFIX "webserv: "

/* error codes */

#define W_NOERR			0
#define W_ERR			1
#define W_SOCKET_ERR	2

/* config */

#define W_DEFAULT_PORT	3000