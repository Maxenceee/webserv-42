#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

char msg[42];
char read_buf[42*4096];
char *array_msg[65536];
int array_fd[65536], sockfd = 0, g_id = 0;

void fatal_error() {
    write(2, "Fatal error\n", 12);
    exit(1);
}

int extract_message(char **buf, char **msg) {
    char *newbuf;
    int i;

    *msg = 0;
    if (*buf == 0)
        return (0);
    i = 0;
    while ((*buf)[i]) {
        if ((*buf)[i] == '\n') {
            newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
            if (newbuf == 0)
                return (-1);
            strcpy(newbuf, *buf + i + 1);
            *msg = *buf;
            (*msg)[i + 1] = 0;
            *buf = newbuf;
            return (1);
        }
        i++;
    }
    return (0);
}

char *str_join(char *buf, char *add) {
    char *newbuf;
    int len;

    if (buf == 0)
        len = 0;
    else
        len = strlen(buf);
    newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
    if (newbuf == 0)
        return (0);
    newbuf[0] = 0;
    if (buf != 0)
        strcat(newbuf, buf);
    free(buf);
    strcat(newbuf, add);
    return (newbuf);
}

void send_all(int except_sock, char *str) {
    int len = strlen(str);
    for (int sock = 0; sock <= sockfd; sock++) {
        if (array_fd[sock] && sock != except_sock)
            send(sock, str, len, 0);
    }
    return ;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        write(2, "Wrong number of arguments\n", 26);
        exit(1);
    }
    int port = atoi(argv[1]);
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    bzero(&addr, addr_len);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = (1 << 24) | 127;
    addr.sin_port = htons(port);

    int server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
        fatal_error();
    sockfd = server;

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server, (const struct sockaddr *)&addr, addr_len) < 0)
        fatal_error();
    if (listen(server, SOMAXCONN) < 0)
        fatal_error();

    bzero(&array_fd, sizeof(array_fd));
    bzero(&read_buf, sizeof(read_buf));
    bzero(&array_msg, sizeof(array_msg));

    struct pollfd fds[65536];
    fds[0].fd = server;
    fds[0].events = POLLIN;

    while (1) {
        int ret = poll(fds, sockfd + 1, -1);
        if (ret == -1) {
            if (errno == EINTR)
                continue;
            else
                fatal_error();
        }
        
        for (int i = 0; i <= sockfd; i++) {
            if (fds[i].revents & POLLIN) {
                printf("ready for pollin %d %d\n", fds[i].fd, server);
                if (fds[i].fd == server) {
                    int client = accept(server, (struct sockaddr *)&addr, &addr_len);
                    if (client < 0)
                        continue;
                    if (client > sockfd)
                        sockfd = client;
                    array_fd[client] = g_id++;
                    bzero(&msg, sizeof(msg));
                    sprintf(msg, "server: client %d just arrived\n", array_fd[client]);
                    send_all(client, msg);
                    fds[client].fd = client;
                    fds[client].events = POLLIN;
                } else {
                    int read_count = recv(fds[i].fd, read_buf, 1000, 0);
                    array_msg[fds[i].fd] = str_join(array_msg[fds[i].fd], read_buf);
                    bzero(&read_buf, sizeof(read_buf));
                    if (read_count <= 0) {
                        bzero(&msg, sizeof(msg));
                        sprintf(msg, "server: client %d just left\n", array_fd[fds[i].fd]);
                        send_all(fds[i].fd, msg);
                        close(fds[i].fd);
                        fds[i].fd = -1;
                    } else {
                        char *tmp;
                        while (extract_message(&array_msg[fds[i].fd], &tmp)){
                            bzero(&msg, sizeof(msg));
                            sprintf(msg, "client %d : ", array_fd[fds[i].fd]);
                            send_all(fds[i].fd, msg);
                            send_all(fds[i].fd, tmp);
                            bzero(&tmp, sizeof(tmp));
                        }
                        bzero(&read_buf, sizeof(read_buf));
                    }
                }
            }
        }
    }
    return 0;
}
