#include "ui_buscar.h"

#include <ncurses.h>
#include <stdio.h>
#include <string.h>

/* Pide el texto a buscar. Devuelve true si se acepto, false si se cancelo
 * (ESC o boton Cancelar). Foco: 0 = campo, 1 = [ Buscar ], 2 = [ Cancelar ] */
static bool pedir_termino(CriterioBusqueda crit, char *query_out, size_t query_size) {
    char texto[MAX_TEXTO] = {0};
    int foco = 0;

    while (1) {
        int alto, ancho;
        getmaxyx(stdscr, alto, ancho);
        int largo_campo = MAX_TEXTO - 1;
        if (largo_campo > ancho - 8) largo_campo = ancho - 8;
        if (largo_campo < 8) largo_campo = 8;

        erase();
        box(stdscr, 0, 0);
        mvprintw(1, 2, "Buscar por -> %s", nombre_criterio(crit));
        mvhline(2, 1, ACS_HLINE, ancho - 2);
        mvprintw(4, 4, "Ingrese termino de busqueda:");

        /* Campo de texto (se muestra el final si el texto es largo) */
        int len = (int)strlen(texto);
        int inicio = len > largo_campo - 1 ? len - (largo_campo - 1) : 0;
        char visible[MAX_TEXTO + 2];
        snprintf(visible, sizeof(visible), "%s%s", texto + inicio, foco == 0 ? "_" : "");

        attron(foco == 0 ? (A_REVERSE | A_BOLD) : A_UNDERLINE);
        mvprintw(6, 4, "%-*.*s", largo_campo, largo_campo, visible);
        attroff(A_REVERSE | A_BOLD | A_UNDERLINE);

        if (foco == 1) attron(A_REVERSE | A_BOLD);
        mvprintw(9, 4, "[ Buscar ]");
        if (foco == 1) attroff(A_REVERSE | A_BOLD);

        if (foco == 2) attron(A_REVERSE | A_BOLD);
        mvprintw(9, 17, "[ Cancelar ]");
        if (foco == 2) attroff(A_REVERSE | A_BOLD);

        mvhline(alto - 2, 1, ACS_HLINE, ancho - 2);
        mvprintw(alto - 1, 2, "ENTER: buscar | Flechas/TAB: mover | ESC: cancelar");
        refresh();

        int ch = getch();
        if (ch == TECLA_ESC) {
            return false;
        } else if (ch == KEY_DOWN || ch == '\t') {
            foco = (foco + 1) % 3;
        } else if (ch == KEY_UP) {
            foco = (foco + 2) % 3;
        } else if ((ch == KEY_LEFT || ch == KEY_RIGHT) && foco > 0) {
            foco = (foco == 1) ? 2 : 1;
        } else if (ch == '\n' || ch == KEY_ENTER) {
            if (foco == 2) return false;
            if (validar_texto(texto, query_out, query_size)) return true;
            mostrar_mensaje_ui("Texto invalido o vacio.");
        } else if (foco == 0) {
            if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
                /* borra el caracter completo aunque sea UTF-8 (acentos) */
                while (len > 0 && (texto[len - 1] & 0xC0) == 0x80) texto[--len] = '\0';
                if (len > 0) texto[--len] = '\0';
            } else if (ch >= 32 && ch <= 255 && ch != 127 && len < MAX_TEXTO - 1) {
                texto[len] = (char)ch;
                texto[len + 1] = '\0';
            }
        }
    }
}

bool pantalla_buscar_por(const CancionUI *catalogo, int total_catalogo,
                       CancionUI *salida, int *total_salida,
                       char *etiqueta, size_t etiqueta_len) {
    const char *opciones[] = {
        "Nombre",
        "Artista",
        "Album",
        "Genero",
        "Cancelar"
    };
    const int num_ops = 5;
    int sel = 0;

    while (1) {
        int alto, ancho;
        getmaxyx(stdscr, alto, ancho);
        erase();
        box(stdscr, 0, 0);

        mvprintw(1, 2, "Buscar por");
        mvhline(2, 1, ACS_HLINE, ancho - 2);

        for (int i = 0; i < num_ops; i++) {
            if (i == sel) attron(A_REVERSE | A_BOLD);
            mvprintw(4 + i * 2, 4, "%-14s", opciones[i]);
            if (i == sel) attroff(A_REVERSE | A_BOLD);
        }

        mvhline(alto - 2, 1, ACS_HLINE, ancho - 2);
        mvprintw(alto - 1, 2, "Flechas arriba/abajo | ENTER seleccionar | ESC cancelar");
        refresh();

        int ch = getch();
        if (ch == KEY_UP) {
            sel = (sel - 1 + num_ops) % num_ops;
        } else if (ch == KEY_DOWN) {
            sel = (sel + 1) % num_ops;
        } else if (ch == TECLA_ESC || ((ch == '\n' || ch == KEY_ENTER) && sel == 4)) {
            return false;
        } else if (ch == '\n' || ch == KEY_ENTER) {
            CriterioBusqueda crit = (CriterioBusqueda)sel;
            char query[MAX_TEXTO] = {0};

            /* Si cancela aqui, regresa al menu "Buscar por" */
            if (!pedir_termino(crit, query, sizeof(query))) {
                continue;
            }

            int n = 0;
            for (int i = 0; i < total_catalogo && n < MAX_CANCIONES; i++) {
                const char *campo = catalogo[i].titulo;
                if (crit == CRITERIO_ARTISTA) campo = catalogo[i].artista;
                else if (crit == CRITERIO_ALBUM) campo = catalogo[i].album;
                else if (crit == CRITERIO_GENERO) campo = catalogo[i].genero;

                if (contiene_texto(campo, query)) {
                    salida[n++] = catalogo[i];
                }
            }

            if (n == 0) {
                mostrar_mensaje_ui("Sin resultados para esa busqueda.");
                continue;
            }

            *total_salida = n;
            snprintf(etiqueta, etiqueta_len, "%s: %s", nombre_criterio(crit), query);
            return true;
        }
    }
}