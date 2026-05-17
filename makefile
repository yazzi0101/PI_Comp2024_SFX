CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -D_GNU_SOURCE -I./include
LDFLAGS = -lpthread -lpq

# Lista explícita de objetos para garantir o rastreamento incremental
OBJS = src/core/system_limits.o \
       src/core/whitelist_parser.o \
       src/core/socket_setup.o \
       src/core/epoll_setup.o \
       src/core/http_parser.o \
       src/core/page_responder.o \
       src/core/ipc_bridge.o \
       src/core/init_server.o \
       src/main.o

TARGET = sfx_server

# Alvo padrão executado ao digitar apenas 'make'
all: $(TARGET)

# Regra de Linkagem: Une todos os arquivos .o no executável final
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Regra Genérica: Compila qualquer arquivo .c em seu respectivo .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Atalho utilitário para limpar os binários antigos
clean:
	rm -f src/core/*.o src/*.o $(TARGET)

.PHONY: all clean
