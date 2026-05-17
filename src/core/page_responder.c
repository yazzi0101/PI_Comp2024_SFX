#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/socket.h>

/**
 * Envia uma resposta HTTP 404 padronizada.
 */
static void send_404_error(int32_t client_fd) {
    const char *not_found =
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 38\r\n"
    "Connection: close\r\n\r\n"
    "<h1>404</h1><p>Pagina nao encontrada.</p>";
    write(client_fd, not_found, strlen(not_found));
}

/**
 * Responde ao cliente web utilizando a system call sendfile (Zero-Copy).
 */
void handle_page_route(int32_t client_fd, const char *filepath) {
    int32_t file_fd = open(filepath, O_RDONLY);
    if (file_fd == -1) {
        send_404_error(client_fd);
        return;
    }

    struct stat st;
    fstat(file_fd, &st);

    /* Mapeamento de MIME types básicos */
    const char *content_type = "text/html";
    if (strstr(filepath, ".js")) content_type = "application/javascript";
    else if (strstr(filepath, ".css")) content_type = "text/css";

    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n\r\n",
             content_type, st.st_size);

    write(client_fd, header, strlen(header));

    /* Kernel move os bytes direto do cache de disco para o socket */
    sendfile(client_fd, file_fd, NULL, st.st_size);
    close(file_fd);
}
