#define _POSIX_C_SOURCE 200809L
#include "biblioteca.h"
#include "archivos.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// datos necesarios para buscar texto en los archivos
typedef struct ContextoBusquedaDisco {
    const char *texto_normalizado;
    ListaArchivos *resultados;
    size_t archivos_revisados;
} ContextoBusquedaDisco;

// datos necesarios para mostrar el catalogo
typedef struct ContextoListado {
    size_t contador;
    size_t limite;
} ContextoListado;

// agrega los libros encontrados al arbol AVL
static void callback_indexar_avl(const char *nombre_archivo,
                                 const char *ruta_completa,
                                 void *contexto) {
    ArbolAVL *avl = (ArbolAVL *)contexto;
    avl_insertar(avl, nombre_archivo, ruta_completa);
}

// busca el texto dentro de un libro
// si lo encuentra, agrega el nombre a la lista de resultados
static void callback_buscar_en_libro(const NodoAVL *nodo, void *contexto) {
    ContextoBusquedaDisco *ctx = (ContextoBusquedaDisco *)contexto;
    ctx->archivos_revisados++;

    if (archivos_contiene_texto(nodo->ruta_completa, ctx->texto_normalizado, true)) {
        lista_archivos_agregar(ctx->resultados, nodo->nombre_archivo);
    }
}

// muestra un libro del catalogo
static void callback_imprimir_catalogo(const NodoAVL *nodo, void *contexto) {
    ContextoListado *ctx = (ContextoListado *)contexto;
    if (ctx->limite > 0 && ctx->contador >= ctx->limite) {
        return;
    }
    ctx->contador++;
    printf("  %zu. %s  (%s)\n", ctx->contador, nodo->nombre_archivo, nodo->ruta_completa);
}

// convierte un texto a minusculas
// quien use esta funcion debe liberar la memoria despues
static char *duplicar_minusculas(const char *texto) {
    if (!texto) return NULL;
    size_t len = strlen(texto);
    char *copia = (char *)malloc(len + 1);
    if (!copia) return NULL;

    for (size_t i = 0; i < len; i++) {
        copia[i] = (char)tolower((unsigned char)texto[i]);
    }
    copia[len] = '\0';
    return copia;
}

// muestra los nombres de los libros encontrados
static void imprimir_lista_resultados(const ListaArchivos *lista, const char *termino) {
    printf("\nResultados para \"%s\" (%zu libro(s) encontrado(s)):\n",
           termino, lista->cantidad);

    if (lista->cantidad == 0) {
        printf("  (Ningun libro contiene el texto buscado)\n");
        return;
    }

    const NodoArchivo *actual = lista->cabeza;
    size_t idx = 1;
    while (actual != NULL) {
        printf("  [%zu] %s\n", idx++, actual->nombre_archivo);
        actual = actual->siguiente;
    }
}

// crea el indice AVL recorriendo la carpeta del corpus
ArbolAVL *biblioteca_construir_indice_avl(const char *ruta_corpus) {
    if (!ruta_corpus) return NULL;

    ArbolAVL *avl = avl_crear();
    if (!avl) return NULL;

    int total = archivos_recorrer_directorio(ruta_corpus, callback_indexar_avl, avl);
    if (total < 0) {
        avl_destruir(avl);
        return NULL;
    }

    return avl;
}

// crea una biblioteca y construye su propio indice AVL
Biblioteca *biblioteca_crear(const char *nombre_comuna, const char *ruta_corpus) {
    if (!ruta_corpus) return NULL;

    ArbolAVL *avl = biblioteca_construir_indice_avl(ruta_corpus);
    if (!avl) return NULL;

    Biblioteca *bib = biblioteca_crear_con_indice(nombre_comuna, avl);
    if (!bib) {
        avl_destruir(avl);
        return NULL;
    }

    bib->es_duena_indice = true;
    return bib;
}

// crea una biblioteca usando un indice AVL existente
Biblioteca *biblioteca_crear_con_indice(const char *nombre_comuna, ArbolAVL *indice_compartido) {
    if (!indice_compartido) return NULL;

    Biblioteca *bib = (Biblioteca *)malloc(sizeof(Biblioteca));
    if (!bib) return NULL;

    bib->nombre_comuna = strdup(nombre_comuna ? nombre_comuna : "Comuna Sin Nombre");
    bib->cache_textos = splay_crear();

    if (!bib->nombre_comuna || !bib->cache_textos) {
        free(bib->nombre_comuna);
        splay_destruir(bib->cache_textos);
        free(bib);
        return NULL;
    }

    bib->indice_libros = indice_compartido;
    bib->es_duena_indice = false;
    bib->cache_hits = 0;
    bib->cache_misses = 0;
    return bib;
}

// busca un libro por su nombre exacto
// si lo encuentra, muestra su ruta y un fragmento si se solicita
bool biblioteca_buscar_libro(const Biblioteca *bib,
                             const char *nombre_archivo,
                             size_t lineas_preview) {
    if (!bib || !bib->indice_libros || !nombre_archivo) return false;

    const NodoAVL *encontrado = avl_buscar_exacto(bib->indice_libros, nombre_archivo);
    if (!encontrado) {
        printf("[AVL - %s] El libro \"%s\" no existe en la biblioteca.\n",
               bib->nombre_comuna, nombre_archivo);
        return false;
    }

    printf("[AVL - %s] Libro encontrado: \"%s\" -> Ruta: %s\n",
           bib->nombre_comuna, encontrado->nombre_archivo, encontrado->ruta_completa);

    if (lineas_preview > 0) {
        archivos_mostrar_fragmento(encontrado->ruta_completa, lineas_preview);
    }

    return true;
}

// busca un texto dentro de los libros
// primero revisa si el resultado ya esta guardado en cache
// si no esta, busca en los archivos y guarda el resultado
int biblioteca_buscar_texto(Biblioteca *bib,
                            const char *texto_buscado,
                            bool mostrar_traza_splay) {
    if (!bib || !bib->cache_textos || !bib->indice_libros ||
        !texto_buscado || texto_buscado[0] == '\0') {
        return -1;
    }

    char *termino_norm = duplicar_minusculas(texto_buscado);
    if (!termino_norm) return -1;

    printf("\n--- Busqueda de texto en Biblioteca (%s): \"%s\" ---\n",
           bib->nombre_comuna, termino_norm);

    // revisa primero si el texto esta en el arbol chaflaneado
    NodoSplay *en_cache = splay_buscar(bib->cache_textos, termino_norm, mostrar_traza_splay);
    if (en_cache != NULL) {
        bib->cache_hits++;
        printf("[CACHE HIT] Resultado recuperado desde el Arbol Chaflaneado (accesos: %zu).\n",
               en_cache->accesos);
        imprimir_lista_resultados(&en_cache->resultados, termino_norm);
        int total = (int)en_cache->resultados.cantidad;
        free(termino_norm);
        return total;
    }

    // si no esta en cache, busca en los libros del disco
    bib->cache_misses++;
    printf("[CACHE MISS] Buscando \"%s\" en %zu libros en disco...\n",
           termino_norm, bib->indice_libros->total_nodos);

    ListaArchivos encontrados;
    lista_archivos_inicializar(&encontrados);

    ContextoBusquedaDisco ctx = {
        .texto_normalizado = termino_norm,
        .resultados = &encontrados,
        .archivos_revisados = 0
    };

    avl_recorrer_inorder(bib->indice_libros, callback_buscar_en_libro, &ctx);

    int total_hallados = (int)encontrados.cantidad;
    imprimir_lista_resultados(&encontrados, termino_norm);

    // guarda el resultado en el arbol chaflaneado
    splay_insertar(bib->cache_textos, termino_norm, &encontrados, mostrar_traza_splay);

    free(termino_norm);
    return total_hallados;
}

// muestra el estado de la cache y sus estadisticas
void biblioteca_mostrar_estado_cache(const Biblioteca *bib) {
    if (!bib) return;
    printf("\nBiblioteca: %s | Libros indexados (AVL): %zu (altura: %d) | Hits: %zu | Misses: %zu\n",
           bib->nombre_comuna,
           bib->indice_libros ? bib->indice_libros->total_nodos : 0,
           avl_obtener_altura(bib->indice_libros),
           bib->cache_hits,
           bib->cache_misses);
    splay_imprimir_arbol(bib->cache_textos);
}

// muestra el catalogo de libros en orden alfabetico
void biblioteca_listar_catalogo(const Biblioteca *bib, size_t limite_impresion) {
    if (!bib || !bib->indice_libros) return;
    printf("\n=== Catalogo de Libros en %s (Total: %zu) ===\n",
           bib->nombre_comuna, bib->indice_libros->total_nodos);

    ContextoListado ctx = { .contador = 0, .limite = limite_impresion };
    avl_recorrer_inorder(bib->indice_libros, callback_imprimir_catalogo, &ctx);

    if (limite_impresion > 0 && bib->indice_libros->total_nodos > limite_impresion) {
        printf("  ... (%zu libros adicionales omitidos)\n",
               bib->indice_libros->total_nodos - limite_impresion);
    }
}

// libera la memoria de la biblioteca y sus estructuras
void biblioteca_destruir(Biblioteca *bib) {
    if (!bib) return;
    free(bib->nombre_comuna);
    splay_destruir(bib->cache_textos);
    if (bib->es_duena_indice && bib->indice_libros) {
        avl_destruir(bib->indice_libros);
    }
    free(bib);
}