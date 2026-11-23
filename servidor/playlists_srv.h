#ifndef PLAYLISTS_SRV_H
#define PLAYLISTS_SRV_H

#include "protocolo.h"
#include "sesion.h"

void srv_ver_playlists(Mensaje *m, const Sesion *s);
void srv_crear_playlist(Mensaje *m, const Sesion *s);
void srv_eliminar_playlist(Mensaje *m, const Sesion *s);
void srv_ver_canciones_pl(Mensaje *m, const Sesion *s);
void srv_agregar_a_pl(Mensaje *m, const Sesion *s);
void srv_quitar_de_pl(Mensaje *m, const Sesion *s);

#endif