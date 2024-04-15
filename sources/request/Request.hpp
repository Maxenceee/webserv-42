/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:15 by mgama             #+#    #+#             */
/*   Updated: 2024/04/15 14:17:20 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webserv.hpp"
#include "server/Server.hpp"

#define WBS_REQ_SUCCESS		0
#define WBS_REQ_ERROR		1

enum ChunkProcessResult {
	WBS_CHUNK_PROCESS_OK,        // Le traitement du chunk s'est bien déroulé
	WBS_CHUNK_PROCESS_ZERO,      // Un chunk de taille 0 a été rencontré
	WBS_CHUNK_PROCESS_ERROR      // Une erreur s'est produite pendant le traitement du chunk
};

class Server;

class Request
{
private:
	bool						_request_line_received;
	bool						_headers_received;
	bool						_body_received;
	int							_status;
	const int					_socket;
	std::string					_version;
	std::string					_method;
	std::string					_path;
	std::string					_raw_path;
	std::string					_host;
	int							_port;
	t_mapss						_query;
	t_mapss						_headers;
	t_mapss						_cookie;
	const sockaddr_in			_clientAddr;
	bool						_transfert_encoding;
	std::string					_body;
	size_t						_body_size;
	std::string					_ip;

	time_t						request_time;

	int		getRequestLine(const std::string &str);
	int		getRequestPath(const std::string &str);
	int		getRequestVersion(const std::string &str);
	int		getRequestQuery(void);
	int		getRequestHostname(const std::string &host);
	int		getRequestCookies(void);

	ChunkProcessResult	processChunk(const std::string &chunks);

public:
	Request(int socket, sockaddr_in clientAddr);
	~Request(void);

	int						processLine(const std::string &line);
	bool					processFinished(void) const;

	bool					headerReceived(void) const;
	bool					bodyReceived(void) const;

	const std::string		&getMethod(void) const;
	const std::string		&getVersion(void) const;
	const std::string		&getPath(void) const;
	const std::string		&getRawPath(void) const;
	int						getPort(void) const;
	const std::string		&getHost(void) const;
	const t_mapss			&getQueries(void) const;
	const std::string		getQueryString(void) const;
	const t_mapss			&getHeaders(void) const;
	const std::string		&getHeader(const std::string name) const;
	const t_mapss			&getCookies(void) const;
	const std::string		&getBody(void) const;
	int						getStatus(void) const;
	int						getClientSocket(void) const;
	const sockaddr_in		&getClientAddr(void) const;
	const std::string		&getIP(void) const;
	time_t					getRequestTime(void) const;
};

std::ostream	&operator<<(std::ostream &os, const Request &req);

#endif /* REQUEST_HPP */