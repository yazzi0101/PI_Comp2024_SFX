#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "http_types.h"

/**
 * Transforma o texto bruto do protocolo HTTP em um Model estruturado.
 * Baseado na análise de cabeçalhos do Capítulo 11 do CS:APP.
 */
int32_t parse_http_request(const char *raw_buffer, HttpRequest *req) {
    if (!raw_buffer || !req) {
        return -1;
    }

    /* Limpa a struct na memória antes do preenchimento (Garantia defensiva) */
    memset(req, 0, sizeof(HttpRequest));

    /* sscanf captura apenas o Método e o Path da primeira linha do HTTP */
    if (sscanf(raw_buffer, "%15s %255s", req->method, req->path) < 2) {
        return -1; /* Request malformado ou incompleto */
    }

    return 0; /* Sucesso */
}
