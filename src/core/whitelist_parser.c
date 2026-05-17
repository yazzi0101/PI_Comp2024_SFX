#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "route_types.h"

#define MAX_ROUTES 100
#define MAX_LINE_LEN 256

Route routing_table[MAX_ROUTES];
int32_t route_count = 0;

static void trim_spaces(char *out, const char *in) {
    while (*in == ' ' || *in == '\t') in++;
    size_t len = strlen(in);
    while (len > 0 && (in[len - 1] == ' ' || in[len - 1] == '\t' || in[len - 1] == '\r' || in[len - 1] == '\n')) {
        len--;
    }
    strncpy(out, in, len);
    out[len] = '\0';
}

int32_t load_whitelist_dsl(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file) && route_count < MAX_ROUTES) {
        /* Avança o ponteiro ignorando qualquer espaço ou tabulação inicial */
        char *start = line;
        while (*start == ' ' || *start == '\t') start++;

        /* Agora a checagem de linha vazia ou comentário funciona mesmo com espaços antes */
        if (*start == '\0' || *start == '#' || *start == '\n' || *start == '\r') {
            continue;
        }

        char *equal_sign = strchr(start, '=');
        if (!equal_sign) continue;

        *equal_sign = '\0';
        char left_side[128], right_side[128];

        trim_spaces(left_side, start);
        trim_spaces(right_side, equal_sign + 1);

        char method[8], path[128];
        /* sscanf ignora nativamente múltiplos espaços entre o método e o path */
        if (sscanf(left_side, "%7s %127s", method, path) != 2) continue;

        strncpy(routing_table[route_count].method, method, 7);
        routing_table[route_count].method[7] = '\0';

        strncpy(routing_table[route_count].path, path, 127);
        routing_table[route_count].path[127] = '\0';

        strncpy(routing_table[route_count].target, right_side, 127);
        routing_table[route_count].target[127] = '\0';

        if (strstr(right_side, ".sock") != NULL) {
            routing_table[route_count].type = ROUTE_SERVICE;
        } else {
            routing_table[route_count].type = ROUTE_PAGE;
        }
        route_count++;
    }
    fclose(file);
    return 0;
}
