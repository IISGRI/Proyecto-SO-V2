#include <ctype.h>
#include <ncurses.h>
#include <stdio.h>
#include <string.h>

#include "interfaz.h"
#include "protocolo.h"
#include "sesion_ui.h"

typedef struct {
    const char *etiqueta;
    char       *valor;
    int         max;
    int         oculto;
} Campo;

typedef int (*Validador)(const Campo *campos, char *error, size_t tam);

/* ------------------------------------------------------------------ */
/*  Validaciones (reglas de tu modulo + las de las notas del equipo)  */
/* ------------------------------------------------------------------ */

int validarCorreo(const char *correo)
{
    const char *arroba = strchr(correo, '@');
    const char *punto;
    int len = (int)strlen(correo);

    if (len < 5 || arroba == NULL || arroba == correo) {
        return 0;
    }
    if (strchr(correo, ' ') != NULL || strchr(correo, ';') != NULL) {
        return 0;
    }
    if (strchr(arroba + 1, '@') != NULL) {
        return 0;
    }
    punto = strchr(arroba + 1, '.');
    if (punto == NULL || punto == arroba + 1) {
        return 0;
    }
    if (*(punto + 1) == '\0' || correo[len - 1] == '.') {
        return 0;
    }
    return 1;
}

/* 8+ caracteres, mayuscula, minuscula, numero y simbolo */
int validarContrasenaSegura(const char *pass)
{
    int mayus = 0, minus = 0, numero = 0, simbolo = 0;
    int len = (int)strlen(pass);
    int i;

    if (len < 8) {
        return 0;
    }
    for (i = 0; i < len; i++) {
        unsigned char c = (unsigned char)pass[i];

        if (isupper(c)) {
            mayus = 1;
        } else if (islower(c)) {
            minus = 1;
        } else if (isdigit(c)) {
            numero = 1;
        } else if (!isspace(c)) {
            simbolo = 1;
        }
    }
    return mayus && minus && numero && simbolo;
}

static int usuario_ok(const char *u)
{
    size_t n = strlen(u);
    size_t i;

    if (n < 3 || n >= LON_USUARIO) {
        return 0;
    }
    for (i = 0; i < n; i++) {
        unsigned char c = (unsigned char)u[i];
        if (!isalnum(c) && c != '_' && c != '-') {
            return 0;
        }
    }
    return 1;
}

static int validar_login(const Campo *c, char *error, size_t tam)
{
    if (c[0].valor[0] == '\0' || c[1].valor[0] == '\0') {
        snprintf(error, tam, "Completa usuario y contrasena");
        return 0;
    }
    return 1;
}

static int validar_registro(const Campo *c, char *error, size_t tam)
{
    if (!usuario_ok(c[0].valor)) {
        snprintf(error, tam, "Usuario: 3-20 letras, numeros, _ o -");
        return 0;
    }
    if (!validarCorreo(c[1].valor)) {
        snprintf(error, tam, "Correo invalido (debe llevar @ y .algo)");
        return 0;
    }
    if (!validarContrasenaSegura(c[2].valor)) {
        snprintf(error, tam, "Contrasena: 8+, Mayus, minus, numero y simbolo");
        return 0;
    }
    if (strcmp(c[2].valor, c[3].valor) != 0) {
        snprintf(error, tam, "Las contrasenas no coinciden");
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Formulario generico                                               */
/* ------------------------------------------------------------------ */

static void dibujar_campo(int fila, int x, int ancho_campo,
                          const Campo *c, int activo)
{
    char visible[80];
    int len = (int)strlen(c->valor);
    int util = ancho_campo - 1;
    int inicio = 0;
    int i, k = 0;
    int atributo = activo ? atributo_seleccion() : (int)A_UNDERLINE;

    if (util > 78) {
        util = 78;
    }
    if (len > util) {
        inicio = len - util;
    }
    for (i = inicio; i < len && k < util; i++) {
        visible[k++] = c->oculto ? '*' : c->valor[i];
    }
    if (activo && len < c->max && k < util) {
        visible[k++] = '_';
    }
    visible[k] = '\0';

    attron(atributo);
    mvprintw(fila, x, "%-*s", ancho_campo, visible);
    attroff(atributo);
}

/* Botones al final: [ boton ] y [ Volver ]. ESC tambien vuelve.
 * Devuelve 1 si se envio el formulario, 0 si se volvio atras. */
static int formulario(const char *titulo, const char *subtitulo,
                      Campo *campos, int n, const char *boton,
                      Validador validar, int activo_inicial)
{
    int activo = activo_inicial;
    char error[100] = "";

    for (;;) {
        int filas, columnas, ancho = 64, alto = n * 2 + 9;
        int y, x, i, tecla, ancho_campo;
        char texto_boton[40];

        getmaxyx(stdscr, filas, columnas);
        if (ancho > columnas - 2) {
            ancho = columnas - 2;
        }
        y = (filas - alto) / 2;
        if (y < 0) {
            y = 0;
        }
        x = (columnas - ancho) / 2;
        ancho_campo = ancho - 29;
        if (ancho_campo < 8) {
            ancho_campo = 8;
        }

        erase();
        dibujar_marco(y, x, alto, ancho, titulo);
        attron(A_DIM);
        centrar_texto(y + 1, x, ancho, subtitulo);
        attroff(A_DIM);
        dibujar_separador(y + 2, x, ancho);

        for (i = 0; i < n; i++) {
            mvprintw(y + 4 + i * 2, x + 3, "%-22s", campos[i].etiqueta);
            dibujar_campo(y + 4 + i * 2, x + 26, ancho_campo, &campos[i], activo == i);
        }

        snprintf(texto_boton, sizeof(texto_boton), "  [ %s ]  ", boton);
        {
            const char *texto_volver = "  [ Volver ]  ";
            int largo1 = (int)strlen(texto_boton);
            int largo2 = (int)strlen(texto_volver);
            int xb = x + (ancho - (largo1 + 3 + largo2)) / 2;

            if (activo == n) {
                attron(atributo_seleccion());
            }
            mvprintw(y + 5 + n * 2, xb, "%s", texto_boton);
            if (activo == n) {
                attroff(atributo_seleccion());
            }

            if (activo == n + 1) {
                attron(atributo_seleccion());
            }
            mvprintw(y + 5 + n * 2, xb + largo1 + 3, "%s", texto_volver);
            if (activo == n + 1) {
                attroff(atributo_seleccion());
            }
        }

        if (error[0] != '\0') {
            attron(COLOR_PAIR(PAR_ERROR) | A_BOLD);
            centrar_texto(y + alto - 2, x, ancho, error);
            attroff(COLOR_PAIR(PAR_ERROR) | A_BOLD);
        }

        dibujar_barra_estado("Flechas/TAB: mover   ENTER: elegir/siguiente   ESC: volver");
        refresh();

        tecla = getch();
        error[0] = '\0';

        if (tecla == KEY_UP) {
            activo = (activo - 1 + n + 2) % (n + 2);
        } else if (tecla == KEY_DOWN || tecla == '\t') {
            activo = (activo + 1) % (n + 2);
        } else if ((tecla == KEY_LEFT || tecla == KEY_RIGHT) && activo >= n) {
            activo = (activo == n) ? n + 1 : n;
        } else if (tecla == 27) {
            return 0;
        } else if (tecla == '\n' || tecla == '\r' || tecla == KEY_ENTER) {
            if (activo < n) {
                activo++;
            } else if (activo == n + 1) {
                return 0;
            } else if (validar(campos, error, sizeof(error))) {
                return 1;
            }
        } else if (activo < n) {
            char *valor = campos[activo].valor;
            size_t len = strlen(valor);

            if (tecla == KEY_BACKSPACE || tecla == 127 || tecla == 8) {
                if (len > 0) {
                    valor[len - 1] = '\0';
                }
            } else if (tecla >= 32 && tecla <= 126 && (int)len < campos[activo].max) {
                valor[len] = (char)tecla;
                valor[len + 1] = '\0';
            }
        }
    }
}

int pantalla_login(char *usuario, char *contrasena)
{
    Campo campos[2] = {
        {"Nombre de usuario:", usuario,    LON_USUARIO - 1, 0},
        {"Contrasena:",        contrasena, MAX_CAMPO,       1}
    };

    /* "usuario" conserva lo que traiga (por si se reintenta); la contrasena no */
    contrasena[0] = '\0';
    return formulario("INICIO DE SESION", "Ingresa tus credenciales",
                      campos, 2, "Iniciar sesion", validar_login,
                      usuario[0] != '\0' ? 1 : 0);
}

int pantalla_registro(char *usuario, char *correo, char *contrasena)
{
    char confirmar_pass[MAX_CAMPO + 1] = "";
    Campo campos[4] = {
        {"Nombre de usuario:",    usuario,         LON_USUARIO - 1, 0},
        {"Correo electronico:",   correo,          LON_CORREO - 1,  0},
        {"Contrasena:",           contrasena,      MAX_CAMPO,       1},
        {"Confirmar contrasena:", confirmar_pass,  MAX_CAMPO,       1}
    };
    int r;

    /* usuario y correo se conservan si se reintenta; las contrasenas no */
    contrasena[0] = '\0';
    r = formulario("REGISTRO DE USUARIO",
                   "Contrasena: 8+ caracteres, Mayus, minus, numero y simbolo",
                   campos, 4, "Registrar", validar_registro, 0);
    memset(confirmar_pass, 0, sizeof(confirmar_pass));
    return r;
}