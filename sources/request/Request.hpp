/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/04 19:01:15 by mgama             #+#    #+#             */
/*   Updated: 2024/06/20 21:07:22 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webserv.hpp"
#include "server/Server.hpp"

#define WBS_REQ_SUCCESS		0
#define WBS_REQ_ERROR		1

enum ChunkProcessResult {
	WBS_CHUNK_PROCESS_OK	= 0x00,		// Le traitement du chunk s'est bien déroulé
	WBS_CHUNK_PROCESS_ZERO	= 0x01,		// Un chunk de taille 0 a été rencontré
	WBS_CHUNK_PROCESS_ERROR	= 0x02		// Une erreur s'est produite pendant le traitement du chunk
};

class Server;

struct wbs_request_time {
	time_t		header;
	time_t		body;
};

class Request
{
private:
	std::string			_raw;
	bool				_request_line_received;
	bool				_headers_received;
	bool				_body_received;
	int					_status;
	const int			_socket;
	std::string			_version;
	std::string			_method;
	std::string			_path;
	std::string			_raw_path;
	std::string			_host;
	int					_port;
	wbs_mapss_t			_query;
	wbs_mapss_t			_headers;
	wbs_mapss_t			_cookie;
	const sockaddr_in	_clientAddr;
	bool				_transfert_encoding;
	std::string			_body;
	size_t				_body_size;
	std::string			_ip;
	std::string			_chunkBuffer;

	struct wbs_request_time		request_time;

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

	bool					headersReceived(void) const;
	bool					bodyReceived(void) const;
	bool					hasContentLength(void) const;
	void					updateHost(const std::string &host);
	void					addHeader(const std::string &header, const std::string &value);
	void					removeHeader(const std::string &header);

	const std::string		&getMethod(void) const;
	const std::string		&getVersion(void) const;
	const std::string		&getPath(void) const;
	const std::string		&getRawPath(void) const;
	int						getPort(void) const;
	const std::string		&getHost(void) const;
	const wbs_mapss_t		&getQueries(void) const;
	const std::string		getQueryString(void) const;
	const wbs_mapss_t		&getHeaders(void) const;
	bool					hasHeader(const std::string name) const;
	const std::string		&getHeader(const std::string name) const;
	const wbs_mapss_t		&getCookies(void) const;
	const std::string		&getBody(void) const;
	int						getStatus(void) const;
	int						getClientSocket(void) const;
	const sockaddr_in		&getClientAddr(void) const;
	const std::string		&getIP(void) const;
	const std::string		&getRawRequest(void) const;
	
	const struct wbs_request_time	&getRequestTime(void) const;

	const std::string		prepareForProxying(void) const;
};

std::ostream	&operator<<(std::ostream &os, const Request &req);

#endif /* REQUEST_HPP */