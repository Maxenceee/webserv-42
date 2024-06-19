/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Daemon.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 15:29:56 by mgama             #+#    #+#             */
/*   Updated: 2024/06/18 23:13:43 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * INFO:
 * En utilisant launchd et systemd, nous n'avons pas besoin de créer un démon.
 */

#ifndef DAEMON_HPP
#define DAEMON_HPP

#include "webserv.hpp"
#include <sys/un.h>

#define WBS_SOCKET_PATH "/var/run/webserv.sock"

#define WBS_NO_CHDIR          01 /* Don't chdir ("/") */
#define WBS_NO_CLOSE_FILES    02 /* Don't close all open files */
#define WBS_NO_REOPEN_STD_FDS 04 /* Don't reopen stdin, stdout, and stderr to /dev/null */
#define WBS_NO_UMASK0        010 /* Don't do a umask(0) */
#define WBS_MAX_CLOSE       8192 /* Max file descriptors to close if sysconf(_SC_OPEN_MAX) is indeterminate */

// returns 0 on success -1 on error
int become_daemon(int flags);

#endif /* DAEMON_HPP */