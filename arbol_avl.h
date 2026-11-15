#ifndef ARBOL_AVL_H
#define ARBOL_AVL_H

#include <stdbool.h>
#include <stddef.h>

// nodo del arbol AVL
typedef struct NodoAVL {
    char *nombre_archivo;     // nombre del archivo
    char *ruta_completa;      // ruta donde se encuentra el archivo
    int altura;
    struct NodoAVL *izq;
    struct NodoAVL *der;
} NodoAVL;

// estructura principal del arbol AVL
typedef struct ArbolAVL {
    NodoAVL *raiz;
    size_t total_nodos;
} ArbolAVL;

// funcion que se ejecuta al recorrer cada nodo del arbol
typedef void (*CallbackNodoAVL)(const NodoAVL *nodo, void *contexto);

// crea un arbol AVL vacio
ArbolAVL *avl_crear(void);

// inserta un libro en el arbol manteniendo el balance
// si el nombre ya existe no lo vuelve a insertar
// retorna true si se inserto correctamente o false si ocurre un error
bool avl_insertar(ArbolAVL *arbol, const char *nombre_archivo, const char *ruta_completa);

// busca un libro por su nombre exacto
// retorna el nodo encontrado o NULL si no existe
const NodoAVL *avl_buscar_exacto(const ArbolAVL *arbol, const char *nombre_archivo);

// recorre todos los nodos del arbol en orden alfabetico
// ejecuta el callback por cada libro encontrado
void avl_recorrer_inorder(const ArbolAVL *arbol, CallbackNodoAVL callback, void *contexto);

// obtiene la altura del arbol
// retorna 0 si el arbol esta vacio
int avl_obtener_altura(const ArbolAVL *arbol);

// libera la memoria utilizada por el arbol y sus nodos
void avl_destruir(ArbolAVL *arbol);

#endif /* ARBOL_AVL_H */