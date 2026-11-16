#ifndef BIBLIOTECA_H
#define BIBLIOTECA_H

#include "arbol_avl.h"
#include "arbol_chaflaneado.h"
#include <stdbool.h>
#include <stddef.h>

// estructura principal de la biblioteca
typedef struct Biblioteca {
    char *nombre_comuna;            // nombre de la comuna
    ArbolAVL *indice_libros;        // arbol AVL con los libros
    bool es_duena_indice;           // indica si debe liberar el AVL
    ArbolChaflaneado *cache_textos; // arbol para guardar las busquedas de texto
    size_t cache_hits;              // cantidad de busquedas encontradas en cache
    size_t cache_misses;            // cantidad de busquedas que necesitaron leer archivos
} Biblioteca;

// crea un indice AVL recorriendo la carpeta del corpus
ArbolAVL *biblioteca_construir_indice_avl(const char *ruta_corpus);

// crea una biblioteca y construye su propio indice AVL
Biblioteca *biblioteca_crear(const char *nombre_comuna, const char *ruta_corpus);

// crea una biblioteca usando un indice AVL que ya existe
// la biblioteca no libera ese indice al destruirse
Biblioteca *biblioteca_crear_con_indice(const char *nombre_comuna, ArbolAVL *indice_compartido);

// busca un libro por el nombre exacto del archivo
// si lo encuentra, muestra su ruta y un fragmento si se solicita
// retorna true si existe o false si no se encuentra
bool biblioteca_buscar_libro(const Biblioteca *bib,
                             const char *nombre_archivo,
                             size_t lineas_preview);

// busca un texto dentro de los libros usando el arbol chaflaneado y los archivos
// primero revisa si el texto ya esta guardado en cache
// si lo encuentra, muestra los resultados guardados
// si no lo encuentra, busca en los archivos y guarda los resultados en cache
// mostrar_traza_splay permite ver los movimientos del arbol
// retorna la cantidad de libros encontrados o -1 si ocurre un error
int biblioteca_buscar_texto(Biblioteca *bib,
                            const char *texto_buscado,
                            bool mostrar_traza_splay);

// muestra el arbol chaflaneado y las estadisticas de la cache
void biblioteca_mostrar_estado_cache(const Biblioteca *bib);

// muestra los libros del AVL en orden alfabetico
void biblioteca_listar_catalogo(const Biblioteca *bib, size_t limite_impresion);

//para obtener el resultado del nombre del archivo en cierta posicion
const char *biblioteca_obtener_resultado_texto(const Biblioteca *bib, size_t indice);

// libera la memoria de la biblioteca
// tambien libera el AVL si le pertenece
void biblioteca_destruir(Biblioteca *bib);

#endif // BIBLIOTECA_H