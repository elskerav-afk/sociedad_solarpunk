#include "biblioteca.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static void crear_corpus_prueba(void) {
    mkdir("corpus_test", 0755);

    FILE *f1 = fopen("corpus_test/kropotkin.txt", "w");
    if (f1) {
        fprintf(f1, "El apoyo mutuo es un factor de la evolucion.\nLa cooperacion fortalece la comuna.\n");
        fclose(f1);
    }

    FILE *f2 = fopen("corpus_test/malatesta.txt", "w");
    if (f2) {
        fprintf(f2, "La libertad sin solidaridad no construye comunidad.\nOrganizacion horizontal.\n");
        fclose(f2);
    }

    FILE *f3 = fopen("corpus_test/goldman.txt", "w");
    if (f3) {
        fprintf(f3, "La educacion y la lectura liberan el pensamiento critico en la comuna.\n");
        fclose(f3);
    }
}

int main(void) {
    crear_corpus_prueba();

    Biblioteca *bib = biblioteca_crear("Comuna Libertad", "corpus_test");
    if (!bib) {
        fprintf(stderr, "Error inicializando biblioteca\n");
        return 1;
    }

    biblioteca_listar_catalogo(bib, 10);

    /* 1. Búsqueda exacta por nombre en AVL */
    biblioteca_buscar_libro(bib, "kropotkin.txt", 2);
    biblioteca_buscar_libro(bib, "inexistente.txt", 0);

    /* 2. Búsquedas de texto (Cache Misses -> inserción en Splay) */
    biblioteca_buscar_texto(bib, "comuna", true);
    biblioteca_mostrar_estado_cache(bib);

    biblioteca_buscar_texto(bib, "libertad", true);
    biblioteca_mostrar_estado_cache(bib);

    biblioteca_buscar_texto(bib, "evolucion", true);
    biblioteca_mostrar_estado_cache(bib);

    /* 3. Búsqueda repetida (Cache Hit -> "comuna" debe subir a la raíz con rotaciones) */
    biblioteca_buscar_texto(bib, "comuna", true);
    biblioteca_mostrar_estado_cache(bib);

    biblioteca_destruir(bib);
    return 0;
}