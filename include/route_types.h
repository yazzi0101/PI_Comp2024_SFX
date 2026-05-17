#ifndef ROUTE_TYPES_H
#define ROUTE_TYPES_H

/* Definição do Tipo de Rota - Inspirado no Capítulo 2 do Extreme C */
typedef enum {
    ROUTE_PAGE,
    ROUTE_SERVICE
} RouteType;

/* O nosso Model/DTO de Rota */
typedef struct {
    char method[8];
    char path[128];
    char target[128];
    RouteType type;
} Route;

#endif
