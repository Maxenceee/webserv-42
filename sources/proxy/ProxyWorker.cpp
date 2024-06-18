/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProxyWorker.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgama <mgama@student.42lyon.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/15 19:22:21 by mgama             #+#    #+#             */
/*   Updated: 2024/06/19 00:12:24 by mgama            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ProxyWorker.hpp"

ProxyWorker::ProxyWorker(int client, const struct wbs_router_proxy &config, const std::string &buffer):
	_client(client),
	_config(config),
	socket_fd(-1),
	_buffer(buffer)
	// _tid(0)
{
	// std::cout << "ProxyWorker: " << buffer << std::endl;
}

ProxyWorker::~ProxyWorker()
{
	// if (this->_pid > -1)
	// {
	// 	kill(this->_pid, SIGKILL);
	// }
	// if (this->socket_fd > -1)
	// {
	// 	close(this->socket_fd);
	// 	close(this->_client);
	// 	Logger::debug("proxy closed", RESET);
	// }
}

int	ProxyWorker::operator()()
{
	if (this->connect() != WBS_PROXY_OK)
	{
		Logger::error("proxy error: could not connect to backend server", RESET);
		close(this->socket_fd);
		return (WBS_PROXY_ERROR);
	}
	if (::send(this->socket_fd, this->_buffer.c_str(), this->_buffer.size(), 0) < 0)
	{
		perror("send");
		return (WBS_PROXY_ERROR);
	}
	Cluster::pool.enqueueTask(relay_data, this->_client, this->socket_fd);
	Logger::debug("new proxy task pushed to pool", RESET);
	return (WBS_PROXY_OK);
}

int	ProxyWorker::connect()
{
	int option = 1;

	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socket_fd < 0)
	{
		perror("socket");
		return (WBS_PROXY_ERROR);
	}

	if (setsockopt(this->socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1) {
		perror("setsockopt");
		return (WBS_SOCKET_ERR);
	}

	struct sockaddr_in addr;
	bzero(&this->socket_addr, sizeof(this->socket_addr));
	this->socket_addr.sin_family = AF_INET;
	this->socket_addr.sin_port = htons(this->_config.port);
	this->socket_addr.sin_addr.s_addr = htonl(setIPAddress(this->_config.host));

	if (::connect(this->socket_fd, (struct sockaddr *)&this->socket_addr, sizeof(this->socket_addr)) < 0)
	{
		perror("connect");
		return (WBS_PROXY_ERROR);
	}

	return (WBS_PROXY_OK);
}

void relay_data(int client_fd, int backend_fd)
{
    fd_set read_fds, write_fds;
    int max_fd = (client_fd > backend_fd ? client_fd : backend_fd) + 1;
    char client_to_backend_buffer[4096], backend_to_client_buffer[4096];
    ssize_t bytes_read, bytes_sent;
    bool client_data_pending = false, backend_data_pending = false;
    int client_to_backend_bytes = 0, backend_to_client_bytes = 0;

    while (true) {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        FD_SET(client_fd, &read_fds);
        FD_SET(backend_fd, &read_fds);

        if (client_data_pending) {
            FD_SET(backend_fd, &write_fds);
        }
        if (backend_data_pending) {
            FD_SET(client_fd, &write_fds);
        }

        int activity = select(max_fd, &read_fds, &write_fds, nullptr, nullptr);
        if (activity < 0) {
            perror("select");
            break;
        }

        // Check if there's data to read from the client
        if (FD_ISSET(client_fd, &read_fds)) {
            bytes_read = recv(client_fd, client_to_backend_buffer, sizeof(client_to_backend_buffer), 0);
            if (bytes_read <= 0) {
                if (bytes_read == 0) {
                    // printf("client closed connection\n");
                } else {
                    perror("recv from client");
                }
                break;
            }
			// std::cout << "client_to_backend_buffer: " << client_to_backend_buffer << std::endl;
            client_to_backend_bytes = bytes_read;
            client_data_pending = true;
        }

        // Check if there's data to read from the backend
        if (FD_ISSET(backend_fd, &read_fds)) {
            bytes_read = recv(backend_fd, backend_to_client_buffer, sizeof(backend_to_client_buffer), 0);
            if (bytes_read <= 0) {
                if (bytes_read == 0) {
                    // printf("backend closed connection\n");
                } else {
                    perror("recv from backend");
                }
                break;
            }
			// std::cout << "backend_to_client_buffer: " << backend_to_client_buffer << std::endl;
            backend_to_client_bytes = bytes_read;
            backend_data_pending = true;
        }

        // Check if the backend is ready to send data from the client
        if (client_data_pending && FD_ISSET(backend_fd, &write_fds)) {
            bytes_sent = send(backend_fd, client_to_backend_buffer, client_to_backend_bytes, 0);
            if (bytes_sent == -1) {
                perror("send to backend");
                break;
            }
            client_data_pending = false;
            // printf("client to backend: %zd bytes\n", bytes_sent);
        }

        // Check if the client is ready to send data from the backend
        if (backend_data_pending && FD_ISSET(client_fd, &write_fds)) {
            bytes_sent = send(client_fd, backend_to_client_buffer, backend_to_client_bytes, 0);
            if (bytes_sent == -1) {
                perror("send to client");
                break;
            }
            backend_data_pending = false;
            printf("backend to client: %zd bytes\n", bytes_sent);
        }
    }

    close(client_fd);
    close(backend_fd);
	Logger::debug("------------------Proxy task ended-------------------\n", B_YELLOW);
}
