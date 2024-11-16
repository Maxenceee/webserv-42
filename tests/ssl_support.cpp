#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <vector>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define MAX_EVENTS 10
#define PORT 3002
#define MAX_CLIENTS 100

void initializeSSL() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

SSL_CTX* createSSLContext() {
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        std::cerr << "Unable to create SSL context" << std::endl;
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Load certificate and private key files
    if (SSL_CTX_use_certificate_file(ctx, "./webserv.crt", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "./webserv.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

int main() {
    initializeSSL();
    SSL_CTX *ctx = createSSLContext();

    // Create socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    int option = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int)) == -1) {
        perror("setsockopt");
        return (EXIT_FAILURE);
    }

    // Prepare sockaddr_in structure
    struct sockaddr_in socket_addr;
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = INADDR_ANY;
    socket_addr.sin_port = htons(PORT);

    // Bind socket to socket_addr and port
    if (bind(socket_fd, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(socket_fd, MAX_EVENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    std::vector<pollfd> poll_fds;
    std::vector<SSL*> ssl_connections;

    // Add server socket to poll set
    poll_fds.push_back((pollfd){socket_fd, POLLIN, 0});

    while (true) {
        std::vector<int> to_remove;

        if (poll(poll_fds.data(), poll_fds.size(), -1) < 0) {
            perror("Poll failed");
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                if (poll_fds[i].fd == socket_fd) {
                    // Accept new connection
                    int new_socket_fd = accept(socket_fd, NULL, NULL);
                    if (new_socket_fd < 0) {
                        perror("Accept failed");
                        exit(EXIT_FAILURE);
                    }

                    printf("New connection accepted\n");

                    // Create SSL structure for the connection
                    SSL *ssl = SSL_new(ctx);
                    SSL_set_fd(ssl, new_socket_fd);

                    if (SSL_accept(ssl) <= 0) {
                        ERR_print_errors_fp(stderr);
                        close(new_socket_fd);
                        SSL_free(ssl);
                    } else {
                        poll_fds.push_back((pollfd){new_socket_fd, POLLIN, 0});
                        ssl_connections.push_back(ssl);
                    }
                } else {
                    // Find the associated SSL connection
                    SSL *ssl = nullptr;
                    for (size_t j = 0; j < ssl_connections.size(); ++j) {
                        if (SSL_get_fd(ssl_connections[j]) == poll_fds[i].fd) {
                            ssl = ssl_connections[j];
                            break;
                        }
                    }

                    if (ssl) {
                        char buffer[1024] = {0};
                        int valread = SSL_read(ssl, buffer, sizeof(buffer) - 1);
                        if (valread <= 0) {
                            printf("Client disconnected or SSL read failed\n");
                            SSL_shutdown(ssl);
                            close(poll_fds[i].fd);
                            SSL_free(ssl);
                            ssl_connections.erase(std::remove(ssl_connections.begin(), ssl_connections.end(), ssl), ssl_connections.end());
                            to_remove.push_back(i);
                        } else {
                            printf("Received: %s\n", buffer);
                            const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, World!";
                            SSL_write(ssl, response, strlen(response));
                        }
                    }
                }
            }
        }

        for (std::vector<int>::reverse_iterator it = to_remove.rbegin(); it != to_remove.rend(); it++) {
            poll_fds.erase(poll_fds.begin() + *it);
        }
    }

    close(socket_fd);
    SSL_CTX_free(ctx);
    EVP_cleanup();
    return 0;
}
