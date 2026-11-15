#define _POSIX_C_SOURCE 200809L
#include "arbol_avl.h"

#include <stdlib.h>
#include <string.h>

// obtiene el numero mayor entre dos valores
static int max_int(int a, int b) {
    return (a > b) ? a : b;
}

// obtiene la altura de un nodo
static int altura_nodo(const NodoAVL *nodo) {
    return nodo ? nodo->altura : 0;
}

// calcula el balance del nodo
static int factor_balance(const NodoAVL *nodo) {
    if (!nodo) return 0;
    return altura_nodo(nodo->izq) - altura_nodo(nodo->der);
}

// crea un nuevo nodo y guarda el nombre y la ruta del archivo
static NodoAVL *crear_nodo(const char *nombre_archivo, const char *ruta_completa) {
    NodoAVL *nuevo = (NodoAVL *)malloc(sizeof(NodoAVL));
    if (!nuevo) return NULL;

    nuevo->nombre_archivo = strdup(nombre_archivo);
    nuevo->ruta_completa = strdup(ruta_completa);
    if (!nuevo->nombre_archivo || !nuevo->ruta_completa) {
        free(nuevo->nombre_archivo);
        free(nuevo->ruta_completa);
        free(nuevo);
        return NULL;
    }

    nuevo->altura = 1;
    nuevo->izq = NULL;
    nuevo->der = NULL;
    return nuevo;
}

// realiza una rotacion hacia la derecha para balancear el arbol
static NodoAVL *rotar_derecha(NodoAVL *y) {
    NodoAVL *x = y->izq;
    NodoAVL *T2 = x->der;

    x->der = y;
    y->izq = T2;

    y->altura = 1 + max_int(altura_nodo(y->izq), altura_nodo(y->der));
    x->altura = 1 + max_int(altura_nodo(x->izq), altura_nodo(x->der));

    return x;
}

// realiza una rotacion hacia la izquierda para balancear el arbol
static NodoAVL *rotar_izquierda(NodoAVL *x) {
    NodoAVL *y = x->der;
    NodoAVL *T2 = y->izq;

    y->izq = x;
    x->der = T2;

    x->altura = 1 + max_int(altura_nodo(x->izq), altura_nodo(x->der));
    y->altura = 1 + max_int(altura_nodo(y->izq), altura_nodo(y->der));

    return y;
}

// inserta un nuevo nodo y mantiene el arbol balanceado
static NodoAVL *insertar_recursivo(NodoAVL *nodo,
                                   const char *nombre_archivo,
                                   const char *ruta_completa,
                                   bool *insertado) {
    if (!nodo) {
        NodoAVL *nuevo = crear_nodo(nombre_archivo, ruta_completa);
        if (nuevo) *insertado = true;
        return nuevo;
    }

    int cmp = strcmp(nombre_archivo, nodo->nombre_archivo);
    if (cmp < 0) {
        nodo->izq = insertar_recursivo(nodo->izq, nombre_archivo, ruta_completa, insertado);
    } else if (cmp > 0) {
        nodo->der = insertar_recursivo(nodo->der, nombre_archivo, ruta_completa, insertado);
    } else {
        // si el nombre ya existe, no lo vuelve a insertar
        *insertado = false;
        return nodo;
    }

    // actualiza la altura del nodo
    nodo->altura = 1 + max_int(altura_nodo(nodo->izq), altura_nodo(nodo->der));

    // revisa si el nodo necesita balancearse
    int balance = factor_balance(nodo);

    // caso izquierda-izquierda
    if (balance > 1 && strcmp(nombre_archivo, nodo->izq->nombre_archivo) < 0) {
        return rotar_derecha(nodo);
    }

    // caso derecha-derecha
    if (balance < -1 && strcmp(nombre_archivo, nodo->der->nombre_archivo) > 0) {
        return rotar_izquierda(nodo);
    }

    // caso izquierda-derecha
    if (balance > 1 && strcmp(nombre_archivo, nodo->izq->nombre_archivo) > 0) {
        nodo->izq = rotar_izquierda(nodo->izq);
        return rotar_derecha(nodo);
    }

    // caso derecha-izquierda
    if (balance < -1 && strcmp(nombre_archivo, nodo->der->nombre_archivo) < 0) {
        nodo->der = rotar_derecha(nodo->der);
        return rotar_izquierda(nodo);
    }

    return nodo;
}

// recorre el arbol en orden y ejecuta el callback por cada nodo
static void inorder_recursivo(const NodoAVL *nodo, CallbackNodoAVL callback, void *contexto) {
    if (!nodo) return;
    inorder_recursivo(nodo->izq, callback, contexto);
    callback(nodo, contexto);
    inorder_recursivo(nodo->der, callback, contexto);
}

// libera la memoria de todos los nodos del arbol
static void destruir_recursivo(NodoAVL *nodo) {
    if (!nodo) return;
    destruir_recursivo(nodo->izq);
    destruir_recursivo(nodo->der);
    free(nodo->nombre_archivo);
    free(nodo->ruta_completa);
    free(nodo);
}

// crea e inicializa un arbol AVL vacio
ArbolAVL *avl_crear(void) {
    ArbolAVL *arbol = (ArbolAVL *)malloc(sizeof(ArbolAVL));
    if (!arbol) return NULL;
    arbol->raiz = NULL;
    arbol->total_nodos = 0;
    return arbol;
}

// inserta un libro en el arbol y actualiza la cantidad de nodos
bool avl_insertar(ArbolAVL *arbol, const char *nombre_archivo, const char *ruta_completa) {
    if (!arbol || !nombre_archivo || !ruta_completa) return false;

    bool insertado = false;
    arbol->raiz = insertar_recursivo(arbol->raiz, nombre_archivo, ruta_completa, &insertado);
    if (insertado) {
        arbol->total_nodos++;
    }
    return insertado;
}

// busca un libro por su nombre exacto
const NodoAVL *avl_buscar_exacto(const ArbolAVL *arbol, const char *nombre_archivo) {
    if (!arbol || !nombre_archivo) return NULL;

    const NodoAVL *actual = arbol->raiz;
    while (actual != NULL) {
        int cmp = strcmp(nombre_archivo, actual->nombre_archivo);
        if (cmp == 0) {
            return actual;
        } else if (cmp < 0) {
            actual = actual->izq;
        } else {
            actual = actual->der;
        }
    }
    return NULL;
}

// recorre todos los libros en orden alfabetico
void avl_recorrer_inorder(const ArbolAVL *arbol, CallbackNodoAVL callback, void *contexto) {
    if (!arbol || !callback) return;
    inorder_recursivo(arbol->raiz, callback, contexto);
}

// obtiene la altura actual del arbol
int avl_obtener_altura(const ArbolAVL *arbol) {
    if (!arbol) return 0;
    return altura_nodo(arbol->raiz);
}

// libera toda la memoria utilizada por el arbol
void avl_destruir(ArbolAVL *arbol) {
    if (!arbol) return;
    destruir_recursivo(arbol->raiz);
    free(arbol);
}