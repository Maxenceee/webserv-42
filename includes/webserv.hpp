/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/12/30 15:26:17 by mgama             #+#    #+#             */
/*   Updated: 2024/12/01 16:33:10 by mgama            ###   ########.fr       */
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
#include <queue>

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
#include <pthread.h>

// C System Includes
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/select.h>

// C Network Includes
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netdb.h>
#include <arpa/inet.h>

// Open SSL Includes
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "pcolors.hpp"
#include "utils/utils.hpp"

#define WBS_SERVER_NAME "webserv/2.0"

/* text */

#define WBS_PREFIX "webserv: "

/* error codes */

#define WBS_NOERR		0
#define WBS_ERR			1
#define WBS_SOCKET_ERR	2

/* config */

#define WBS_DEFAULT_PORT	3000

#define WBS_RECV_SIZE	2 << 12

#ifdef REQUEST_TIMEOUT
#define WBS_REQUEST_TIMEOUT	REQUEST_TIMEOUT
#else
// default timeout duration, 1 minute in milliseconds
#define WBS_REQUEST_TIMEOUT	60000
#endif /* REQUEST_TIMEOUT */

#ifdef POLL_TIMEOUT
#define WBS_POLL_TIMEOUT	POLL_TIMEOUT
#else
#define WBS_POLL_TIMEOUT	100
#endif /* POLL_TIMEOUT */

#define WBS_DEFAULT_MAX_WORKERS	1024

/* */

#define WBS_CRLF "\r\n"

/* typedef */

// std::map<std::string, std::string>
typedef std::map<std::string, std::string>	wbs_mapss_t;
// std::map<int, std::string>
typedef std::map<int, std::string>			wbs_mapis_t;

#endif /* WEBSERV_HPP */
