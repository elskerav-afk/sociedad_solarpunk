#ifndef ARBOL_CHAFLANEADO_H
#define ARBOL_CHAFLANEADO_H

#include <stdbool.h>
#include <stddef.h>

// nodo de la lista que guarda los archivos donde se encontro un texto
typedef struct NodoArchivo {
    char *nombre_archivo;
    struct NodoArchivo *siguiente;
} NodoArchivo;

// lista de archivos encontrados en una busqueda
typedef struct ListaArchivos {
    NodoArchivo *cabeza;
    NodoArchivo *cola;
    size_t cantidad;
} ListaArchivos;

// nodo del arbol chaflaneado
typedef struct NodoSplay {
    char *termino_buscado;       // texto que se busca en el arbol
    ListaArchivos resultados;    // archivos donde aparece el texto
    size_t accesos;              // cantidad de veces que se ha buscado el texto
    struct NodoSplay *izq;
    struct NodoSplay *der;
} NodoSplay;

// estructura principal del arbol chaflaneado
typedef struct ArbolChaflaneado {
    NodoSplay *raiz;
    size_t total_nodos;
} ArbolChaflaneado;

// inicializa una lista de archivos vacia
void lista_archivos_inicializar(ListaArchivos *lista);

// agrega un archivo a la lista
bool lista_archivos_agregar(ListaArchivos *lista, const char *nombre_archivo);

// libera la memoria utilizada por la lista
void lista_archivos_liberar(ListaArchivos *lista);

// crea un arbol chaflaneado vacio
ArbolChaflaneado *splay_crear(void);

// busca un termino en el arbol y lo mueve hacia la raiz
// si no existe, mueve el ultimo nodo visitado
// permite mostrar los movimientos del arbol en consola
// retorna el nodo encontrado o NULL si no existe
NodoSplay *splay_buscar(ArbolChaflaneado *arbol, const char *termino, bool mostrar_traza);

// inserta un nuevo termino en el arbol y lo coloca en la raiz
// tambien guarda la lista de archivos donde se encontro
// retorna el nodo insertado o actualizado
NodoSplay *splay_insertar(ArbolChaflaneado *arbol,
                          const char *termino,
                          ListaArchivos *resultados,
                          bool mostrar_traza);

// muestra la estructura del arbol chaflaneado en consola
void splay_imprimir_arbol(const ArbolChaflaneado *arbol);

// libera la memoria utilizada por el arbol y sus listas
void splay_destruir(ArbolChaflaneado *arbol);

#endif /* ARBOL_CHAFLANEADO_H */