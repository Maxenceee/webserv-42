/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 15:26:17 by mgama             #+#    #+#             */
/*   Updated: 2024/04/16 20:05:05 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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
#include <cstdarg>
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

// C Includes
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>
#include <poll.h>
#include <regex.h>
#include <signal.h>
#include <errno.h>

// C System Includes
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

// C Network Includes
#include <netinet/in.h>
#include <netinet/ip.h> 

#include "pcolors.hpp"
#include "utils/utils.hpp"

#define WBS_SERVER_NAME "webserv/1.0"

/* text */

#define WBS_PREFIX "webserv: "

/* error codes */

#define WBS_NOERR		0
#define WBS_ERR			1
#define WBS_SOCKET_ERR	2

/* config */

#define WBS_DEFAULT_PORT	3000

#define WBS_RECV_SIZE	2 << 12

#define WBS_REQUEST_TIMEOUT	60000 // 1 minute in milliseconds

/* */

#define WBS_CRLF "\r\n"

/* typedef */
typedef std::map<std::string, std::string> t_mapss;

#endif /* WEBSERV_HPP */