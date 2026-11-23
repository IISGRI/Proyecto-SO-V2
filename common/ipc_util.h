#ifndef IPC_UTIL_H
#define IPC_UTIL_H

#include <stddef.h>

int  sem_fijar(int semid, int num, int valor);
int  sem_up(int semid, int num);
int  sem_down(int semid, int num);            /* 0 = ok, -1 = error        */
int  sem_down_timeout(int semid, int num, int segundos);
                                              /* 0 = ok, 1 = timeout/senal, -1 = error */
void copiar_cadena(char *destino, size_t tam, const char *origen);

#endif