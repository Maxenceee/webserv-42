/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 15:26:17 by mgama             #+#    #+#             */
/*   Updated: 2024/01/08 17:49:30 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// CPP Includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include <ctime>
#include <limits>
#include <cstdio>
#include <filesystem>
#include <numeric>

// CPP Containers
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iterator>
#include <list>
#include <utility>

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <poll.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "pcolors.hpp"
#include "utils/utils.hpp"

/* text */

#define W_PREFIX "webserv: "

/* error codes */

#define W_NOERR			0
#define W_ERR			1
#define W_SOCKET_ERR	2

/* config */

#define W_DEFAULT_PORT	4242

#define RECV_SIZE		65536

/* typedef */
typedef std::map<std::string, std::string> t_mapss;