/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 15:26:17 by mgama             #+#    #+#             */
/*   Updated: 2024/01/04 19:34:17 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// CPP Includes
# include <iostream>
# include <iomanip>
# include <sstream>
# include <fstream>
# include <string>
# include <limits>
# include <cstdio>

// CPP Containers
# include <map>
# include <set>
# include <vector>
# include <algorithm>
# include <iterator>
# include <list>
# include <utility>

#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <sys/stat.h>

#include "pcolors.hpp"

/* text */

#define W_PREFIX "webserv: "

/* error codes */

#define W_NOERR			0
#define W_ERR			1
#define W_SOCKET_ERR	2

/* config */

#define W_DEFAULT_PORT	4242

#define RECV_SIZE		65536