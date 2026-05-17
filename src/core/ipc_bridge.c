#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFFER_SIZE 8192

/**
 * Conecta a um Unix Domain Socket e atua como um pass-through cego de bytes.
 */
void handle_service_route(int32_t client_fd, const char *socket_path, const char *raw_request, size_t request_len) {
    int32_t ipc_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (ipc_fd == -1) {
        const char *err = "HTTP/1.1 500 Internal Error\r\n\r\n";
        write(client_fd, err, strlen(err));
        return;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    /* Tenta o aperto de mão local com o processo do microsserviço */
    if (connect(ipc_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        const char *bad_gateway =
        "HTTP/1.1 502 Bad Gateway\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 41\r\n\r\n"
        "<h1>502</h1><p>Servico Indisponivel.</p>";
        write(client_fd, bad_gateway, strlen(bad_gateway));
        close(ipc_fd);
        return;
    }

    /* Escreve o fluxo bruto recebido da rede no cano do IPC */
    write(ipc_fd, raw_request, request_len);

    /* Lê a resposta do microsserviço e descarrega direto no socket do navegador */
    char ipc_buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = read(ipc_fd, ipc_buffer, sizeof(ipc_buffer))) > 0) {
        write(client_fd, ipc_buffer, bytes_received);
    }

    close(ipc_fd);
}
