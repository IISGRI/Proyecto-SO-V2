#include <stdio.h>
#include "cifrado.h"

void contrasenacif(const char *entrada, char *salida)
{
    unsigned long hashear = 5381;
    int c;

    while ((c = *entrada++)) {
        hashear = ((hashear << 5) + hashear) + (unsigned long)c; /* hash * 33 + c */
    }

    snprintf(salida, 21, "%lu", hashear);
}