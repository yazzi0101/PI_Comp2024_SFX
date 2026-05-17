#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>

#define BACKLOG 4096

static int32_t set_nonblocking(int32_t fd) {
    int32_t flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) return -1;
    return 0;
}

/* Agora aceita o parâmetro host (IP) explicitamente */
int32_t create_and_bind_socket(const char *host, const char *port) {
    struct addrinfo hints, *res, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    /* Se o IP for "0.0.0.0", usamos AI_PASSIVE, caso contrário, amarração estrita */
    if (strcmp(host, "0.0.0.0") == 0) {
        hints.ai_flags = AI_PASSIVE;
        host = NULL;
    }

    if (getaddrinfo(host, port, &hints, &res) != 0) return -1;

    int32_t listen_fd = -1;
    for (p = res; p != NULL; p = p->ai_next) {
        listen_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listen_fd == -1) continue;

        int32_t opt = 1;
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
            close(listen_fd); freeaddrinfo(res); return -1;
        }

        if (bind(listen_fd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(listen_fd);
    }
    freeaddrinfo(res);
    if (p == NULL) return -1;

    if (set_nonblocking(listen_fd) == -1 || listen(listen_fd, BACKLOG) == -1) {
        close(listen_fd); return -1;
    }
    return listen_fd;
}
