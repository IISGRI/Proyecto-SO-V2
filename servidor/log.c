#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "log.h"

static pthread_mutex_t mutex_log = PTHREAD_MUTEX_INITIALIZER;

void log_evento(const char *formato, ...)
{
    char mensaje[512];
    char hora[32];
    char fecha[16];
    char ruta[64];
    time_t ahora = time(NULL);
    struct tm info;
    va_list args;
    FILE *archivo;

    va_start(args, formato);
    vsnprintf(mensaje, sizeof(mensaje), formato, args);
    va_end(args);

    localtime_r(&ahora, &info);
    strftime(hora, sizeof(hora), "%H:%M:%S", &info);
    strftime(fecha, sizeof(fecha), "%Y-%m-%d", &info);
    snprintf(ruta, sizeof(ruta), "datos/log_%s.txt", fecha);

    pthread_mutex_lock(&mutex_log);

    printf("[%s] %s\n", hora, mensaje);
    fflush(stdout);

    archivo = fopen(ruta, "a");
    if (archivo != NULL) {
        fprintf(archivo, "[%s] %s\n", hora, mensaje);
        fclose(archivo);
    }

    pthread_mutex_unlock(&mutex_log);
}