#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H

/* O nosso Model/DTO que representa uma requisição HTTP capturada */
typedef struct {
    char method[16];
    char path[256];
} HttpRequest;

#endif
