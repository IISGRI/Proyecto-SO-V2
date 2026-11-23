#ifndef SESION_UI_H
#define SESION_UI_H

#define MAX_CAMPO 35   /* longitud maxima de la contrasena (sin contar '\0') */

/* usuario: >= 21 bytes, contrasena: >= MAX_CAMPO + 1 bytes
 * "usuario" (y "correo") deben venir inicializados: "" la primera vez, o lo
 * ya escrito si se reintenta. La contrasena siempre se limpia.
 * Devuelven 1 si el usuario envio el formulario, 0 si volvio (ESC o [ Volver ]) */
int pantalla_login(char *usuario, char *contrasena);

/* correo: >= 64 bytes */
int pantalla_registro(char *usuario, char *correo, char *contrasena);

int validarCorreo(const char *correo);
int validarContrasenaSegura(const char *pass);

#endif