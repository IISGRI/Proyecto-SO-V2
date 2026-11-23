#ifndef RESPUESTA_H
#define RESPUESTA_H

#include "protocolo.h"

void resp_ok(Mensaje *m, const char *texto);
void resp_error(Mensaje *m, const char *texto);

/* Agrega una linea al campo "datos". 0 = ok, -1 = no cabe */
int  datos_agregar(Mensaje *m, const char *linea);

#endif