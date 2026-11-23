#include <string.h>

#include "ipc_util.h"
#include "respuesta.h"

void resp_ok(Mensaje *m, const char *texto)
{
    m->codigo = RESP_OK;
    copiar_cadena(m->mensaje, sizeof(m->mensaje), texto);
}

void resp_error(Mensaje *m, const char *texto)
{
    m->codigo = RESP_ERROR;
    copiar_cadena(m->mensaje, sizeof(m->mensaje), texto);
}

int datos_agregar(Mensaje *m, const char *linea)
{
    size_t usado = strlen(m->datos);
    size_t largo = strlen(linea);

    if (usado + largo + 1 >= sizeof(m->datos)) {
        return -1;
    }
    memcpy(m->datos + usado, linea, largo + 1);
    return 0;
}