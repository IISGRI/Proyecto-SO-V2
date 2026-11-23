# ----------------------------------------------------------------
# Prototipo inicial: cliente + servidor (semaforos + memoria compartida)
# ----------------------------------------------------------------
CC     = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -D_GNU_SOURCE -Icommon

COMUN = common/ipc_util.c common/cifrado.c

SRV_SRC = servidor/Servidor.c servidor/usuarios.c servidor/biblioteca.c \
          servidor/playlists_srv.c servidor/respuesta.c servidor/log.c \
          $(COMUN)
SRV_INC = -Iservidor

CLI_SRC = cliente/Cliente.c cliente/comunicacion.c cliente/menus.c \
          cliente/interfaz/interfaz.c cliente/auth/sesion_ui.c \
          cliente/biblioteca/ui_comun.c cliente/biblioteca/ui_buscar.c \
          cliente/biblioteca/ui_biblioteca.c cliente/playlists/playlists.c \
          $(COMUN)
CLI_INC = -Icliente -Icliente/interfaz -Icliente/auth \
          -Icliente/biblioteca -Icliente/playlists

HEADERS = $(wildcard common/*.h servidor/*.h cliente/*.h cliente/*/*.h)

all: archivo Cliente Servidor

# "archivo" es solo para que ftok() genere las llaves de IPC
archivo:
	touch archivo

Cliente: $(CLI_SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(CLI_INC) $(CLI_SRC) -o Cliente -lncurses

Servidor: $(SRV_SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRV_INC) $(SRV_SRC) -o Servidor -lpthread

clean:
	rm -f Cliente Servidor

.PHONY: all clean