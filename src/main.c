#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include "route_types.h"
#include "http_types.h"

/* CONFIGURAÇÕES DO SERVIDOR VIA MACROS — AJUSTE AQUI */
#define IP_ADDRESS "0.0.0.0"
#define PORT       "8080"

#define MAX_EVENTS 64
#define BUFFER_SIZE 8192

extern Route routing_table[];
extern int32_t route_count;

extern int32_t init_server(const char *host, const char *port, int32_t *epoll_fd, const char *whitelist_path);
extern int32_t parse_http_request(const char *raw_buffer, HttpRequest *req);
extern void handle_page_route(int32_t client_fd, const char *filepath);
extern void handle_service_route(int32_t client_fd, const char *socket_path, const char *raw_request, size_t request_len);

static void process_client_request(int32_t client_fd) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) { close(client_fd); return; }

    HttpRequest req;
    if (parse_http_request(buffer, &req) == -1) {
        const char *bad = "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
        write(client_fd, bad, strlen(bad)); close(client_fd); return;
    }

    for (int32_t i = 0; i < route_count; i++) {
        if (strcmp(routing_table[i].method, req.method) == 0 && strcmp(routing_table[i].path, req.path) == 0) {
            if (routing_table[i].type == ROUTE_PAGE) {
                handle_page_route(client_fd, routing_table[i].target);
            } else {
                handle_service_route(client_fd, routing_table[i].target, buffer, bytes_read);
            }
            return;
        }
    }
    const char *not_found = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
    write(client_fd, not_found, strlen(not_found)); close(client_fd);
}
int main(void) {
    int32_t epoll_fd = -1;

    /* Inicializa o servidor tentando ler o routes.txt da raiz */
    int32_t listen_fd = init_server(IP_ADDRESS, PORT, &epoll_fd, "routes.txt");

    /* Se falhar, interroga o Kernel para saber o motivo exato */
    if (listen_fd == -1) {
        fprintf(stderr, "============== ERRO CRÍTICO NO BOOT ==============\n");
        fprintf(stderr, "Motivo relatado pelo Kernel: %s\n", strerror(errno));
        fprintf(stderr, "==================================================\n");
        return EXIT_FAILURE;
    }

    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int32_t num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) { if (errno == EINTR) continue; break; }

        for (int32_t i = 0; i < num_events; i++) {
            if (events[i].data.fd == listen_fd) {
                while (1) {
                    int32_t client_fd = accept(listen_fd, NULL, NULL);
                    if (client_fd == -1) { if (errno == EAGAIN || errno == EWOULDBLOCK) break; break; }

                    int32_t flags = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                    struct epoll_event ev = { .events = EPOLLIN | EPOLLET, .data.fd = client_fd };
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                }
            } else {
                process_client_request(events[i].data.fd);
            }
        }
    }
    close(listen_fd); close(epoll_fd);
    return EXIT_SUCCESS;
}
