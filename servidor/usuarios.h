#ifndef USUARIOS_H
#define USUARIOS_H

#include "protocolo.h"
#include "sesion.h"

int  usuario_valido(const char *usuario);
void srv_registro(Mensaje *m);
void srv_login(Mensaje *m, Sesion *s);
void srv_cerrar_sesion(Mensaje *m, Sesion *s);

#endif