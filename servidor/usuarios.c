#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "ipc_util.h"
#include "respuesta.h"
#include "usuarios.h"

/* Formato de cada linea: usuario;correo;hash */
#define ARCH_USUARIOS "datos/usuarios.txt"

int usuario_valido(const char *usuario)
{
    size_t n = strlen(usuario);
    size_t i;

    if (n < 3 || n >= LON_USUARIO) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)usuario[i];
        if (!isalnum(c) && c != '_' && c != '-') {
            return 0;
        }
    }
    return 1;
}

static int correo_valido(const char *correo)
{
    size_t n = strlen(correo);
    const char *arroba = strchr(correo, '@');
    const char *punto;

    if (n < 5 || n >= LON_CORREO || arroba == NULL || arroba == correo) {
        return 0;
    }
    if (strchr(correo, ' ') != NULL || strchr(correo, ';') != NULL) {
        return 0;
    }
    if (strchr(arroba + 1, '@') != NULL) {
        return 0;
    }
    punto = strchr(arroba + 1, '.');
    if (punto == NULL || punto == arroba + 1 || correo[n - 1] == '.') {
        return 0;
    }
    return 1;
}

static int hash_valido(const char *hash)
{
    size_t n = strlen(hash);
    size_t i;

    if (n == 0 || n > 20) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        if (!isdigit((unsigned char)hash[i])) {
            return 0;
        }
    }
    return 1;
}

/* Parte "usuario;correo;hash" en sus tres campos (modifica la linea) */
static int separar(char *linea, char **usuario, char **correo, char **hash)
{
    char *p;

    *usuario = linea;
    p = strchr(linea, ';');
    if (p == NULL) {
        return 0;
    }
    *p++ = '\0';
    *correo = p;
    p = strchr(p, ';');
    if (p == NULL) {
        return 0;
    }
    *p++ = '\0';
    *hash = p;
    p[strcspn(p, "\r\n")] = '\0';
    return 1;
}

void srv_registro(Mensaje *m)
{
    FILE *archivo;
    char linea[256];
    char *u, *c, *h;
    int existe = 0;

    if (!usuario_valido(m->usuario)) {
        resp_error(m, "Usuario invalido (3-20 letras, numeros, _ o -)");
        return;
    }
    if (!correo_valido(m->correo)) {
        resp_error(m, "Correo invalido");
        return;
    }
    if (!hash_valido(m->hash)) {
        resp_error(m, "Contrasena invalida");
        return;
    }

    archivo = fopen(ARCH_USUARIOS, "r");
    if (archivo != NULL) {
        while (fgets(linea, sizeof(linea), archivo)) {
            if (separar(linea, &u, &c, &h) && strcasecmp(u, m->usuario) == 0) {
                existe = 1;
                break;
            }
        }
        fclose(archivo);
    }

    if (existe) {
        resp_error(m, "Ese nombre de usuario ya esta registrado");
        return;
    }

    archivo = fopen(ARCH_USUARIOS, "a");
    if (archivo == NULL) {
        resp_error(m, "No se pudo abrir el archivo de usuarios");
        return;
    }
    fprintf(archivo, "%s;%s;%s\n", m->usuario, m->correo, m->hash);
    fclose(archivo);

    resp_ok(m, "Cuenta creada correctamente");
}

void srv_login(Mensaje *m, Sesion *s)
{
    FILE *archivo;
    char linea[256];
    char *u, *c, *h;
    int correcto = 0;

    if (s->logueado) {
        resp_error(m, "Ya hay una sesion iniciada");
        return;
    }
    if (!usuario_valido(m->usuario) || !hash_valido(m->hash)) {
        resp_error(m, "Usuario o contrasena incorrectos");
        return;
    }

    archivo = fopen(ARCH_USUARIOS, "r");
    if (archivo == NULL) {
        resp_error(m, "No se pudo abrir el archivo de usuarios");
        return;
    }
    while (fgets(linea, sizeof(linea), archivo)) {
        if (separar(linea, &u, &c, &h) &&
            strcmp(u, m->usuario) == 0 && strcmp(h, m->hash) == 0) {
            correcto = 1;
            break;
        }
    }
    fclose(archivo);

    if (!correcto) {
        resp_error(m, "Usuario o contrasena incorrectos");
        return;
    }

    s->logueado = 1;
    copiar_cadena(s->usuario, sizeof(s->usuario), m->usuario);
    resp_ok(m, "Bienvenido");
}

void srv_cerrar_sesion(Mensaje *m, Sesion *s)
{
    s->logueado = 0;
    s->usuario[0] = '\0';
    resp_ok(m, "Sesion cerrada");
}