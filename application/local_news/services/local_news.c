#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <libpq-fe.h>

#define SOCKET_PATH "/tmp/news.sock"
#define BUFFER_SIZE 8192

static void save_to_db(const char *title, const char *body) {
    /* String de conexão com o Postgres nativo */
    PGconn *conn = PQconnectdb("dbname=sfx_db user=victor_sousa host=localhost");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Erro na conexao com o Postgres: %s\n", PQerrorMessage(conn));
        PQfinish(conn); return;
    }

    /* Prevenção de SQL Injection usando Prepared Statements nativos do libpq */
    const char *paramValues[2] = { title, body };
    PGresult *res = PQexecParams(conn, "INSERT INTO news (title, body) VALUES ($1, $2);",
                                 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Falha na insercao: %s\n", PQerrorMessage(conn));
    }
    PQclear(res); PQfinish(conn);
}

static void handle_gateway_request(int32_t client_fd) {
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) return;

    /* Encontra o início do payload JSON pulando os cabeçalhos HTTP */
    char *json_start = strchr(buffer, '{');
    if (!json_start) return;

    /* Parsing primitivo de strings (KISS) para extrair as propriedades do JSON */
    char title[128] = {0}, body[512] = {0};
    char *t_ptr = strstr(json_start, "\"title\":\"");
    char *b_ptr = strstr(json_start, "\"body\":\"");

    if (t_ptr) sscanf(t_ptr, "\"title\":\"%127[^\"]\"", title);
    if (b_ptr) sscanf(b_ptr, "\"body\":\"%511[^\"]\"", body);

    /* Envia os dados limpos para a persistência no Postgres */
    if (strlen(title) > 0 && strlen(body) > 0) {
        save_to_db(title, body);
    }

    /* Retorna uma resposta HTTP válida que o Gateway vai repassar em formato cego */
    const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    write(client_fd, response, strlen(response));
}

int main(void) {
    int32_t server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH); /* Remove o arquivo de socket antigo se houver */
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1 || listen(server_fd, 10) == -1) {
        perror("Erro ao iniciar socket local_news"); return 1;
    }

    printf("Microsservico local_news aguardando dados no socket %s...\n", SOCKET_PATH);
    while (1) {
        int32_t client_fd = accept(server_fd, NULL, NULL);
        if (client_fd != -1) {
            handle_gateway_request(client_fd);
            close(client_fd);
        }
    }
    close(server_fd); return 0;
}
