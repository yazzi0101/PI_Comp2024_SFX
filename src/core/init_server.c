#include <stdint.h>
#include <stdio.h>

extern int32_t configure_system_limits(void);
extern int32_t load_whitelist_dsl(const char *filename);
extern int32_t create_and_bind_socket(const char *host, const char *port);
extern int32_t setup_epoll(int32_t listen_fd);

int32_t init_server(const char *host, const char *port, int32_t *epoll_fd, const char *whitelist_path) {
    if (configure_system_limits() == -1) return -1;
    if (load_whitelist_dsl(whitelist_path) == -1) return -1;

    int32_t listen_fd = create_and_bind_socket(host, port);
    if (listen_fd == -1) return -1;

    *epoll_fd = setup_epoll(listen_fd);
    if (*epoll_fd == -1) return -1;

    printf("Sucesso: API Gateway rodando em http://%s:%s\n", host, port);
    return listen_fd;
}
