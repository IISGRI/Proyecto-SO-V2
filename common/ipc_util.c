#include <errno.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "ipc_util.h"

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int sem_fijar(int semid, int num, int valor)
{
    union semun arg;

    arg.val = valor;
    return semctl(semid, num, SETVAL, arg);
}

int sem_up(int semid, int num)
{
    struct sembuf op = {(unsigned short)num, 1, 0};

    return semop(semid, &op, 1);
}

int sem_down(int semid, int num)
{
    struct sembuf op = {(unsigned short)num, -1, 0};

    while (semop(semid, &op, 1) == -1) {
        if (errno != EINTR) {
            return -1;
        }
    }
    return 0;
}

int sem_down_timeout(int semid, int num, int segundos)
{
    struct sembuf op = {(unsigned short)num, -1, 0};
    struct timespec limite;

    limite.tv_sec = segundos;
    limite.tv_nsec = 0;

    if (semtimedop(semid, &op, 1, &limite) == 0) {
        return 0;
    }
    if (errno == EAGAIN || errno == EINTR) {
        return 1;
    }
    return -1;
}

void copiar_cadena(char *destino, size_t tam, const char *origen)
{
    size_t i = 0;

    if (tam == 0) {
        return;
    }
    while (origen[i] != '\0' && i + 1 < tam) {
        destino[i] = origen[i];
        i++;
    }
    destino[i] = '\0';
}