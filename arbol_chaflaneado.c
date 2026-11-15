#define _POSIX_C_SOURCE 200809L
#include "arbol_chaflaneado.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// inicializa una lista vacia
void lista_archivos_inicializar(ListaArchivos *lista) {
    if (!lista) return;
    lista->cabeza = NULL;
    lista->cola = NULL;
    lista->cantidad = 0;
}

// agrega un archivo al final de la lista
bool lista_archivos_agregar(ListaArchivos *lista, const char *nombre_archivo) {
    if (!lista || !nombre_archivo) return false;

    NodoArchivo *nuevo = (NodoArchivo *)malloc(sizeof(NodoArchivo));
    if (!nuevo) return false;

    nuevo->nombre_archivo = strdup(nombre_archivo);
    if (!nuevo->nombre_archivo) {
        free(nuevo);
        return false;
    }
    nuevo->siguiente = NULL;

    if (lista->cola) {
        lista->cola->siguiente = nuevo;
    } else {
        lista->cabeza = nuevo;
    }
    lista->cola = nuevo;
    lista->cantidad++;
    return true;
}

// libera la memoria utilizada por la lista
void lista_archivos_liberar(ListaArchivos *lista) {
    if (!lista) return;
    NodoArchivo *actual = lista->cabeza;
    while (actual != NULL) {
        NodoArchivo *sig = actual->siguiente;
        free(actual->nombre_archivo);
        free(actual);
        actual = sig;
    }
    lista->cabeza = NULL;
    lista->cola = NULL;
    lista->cantidad = 0;
}

// realiza una rotacion hacia la derecha
static NodoSplay *rotar_derecha(NodoSplay *x) {
    NodoSplay *y = x->izq;
    x->izq = y->der;
    y->der = x;
    return y;
}

// realiza una rotacion hacia la izquierda
static NodoSplay *rotar_izquierda(NodoSplay *x) {
    NodoSplay *y = x->der;
    x->der = y->izq;
    y->izq = x;
    return y;
}

// busca un termino y lo mueve hacia la raiz del arbol
// si no existe, mueve el ultimo nodo visitado
static NodoSplay *splay_recursivo(NodoSplay *raiz, const char *termino, bool mostrar_traza) {
    if (!raiz) return NULL;

    int cmp = strcmp(termino, raiz->termino_buscado);
    if (cmp == 0) {
        return raiz;
    }

    // busca en el subarbol izquierdo
    if (cmp < 0) {
        if (!raiz->izq) return raiz;

        int cmp_izq = strcmp(termino, raiz->izq->termino_buscado);

        if (cmp_izq < 0) {
            // caso zig-zig: dos rotaciones hacia la derecha
            raiz->izq->izq = splay_recursivo(raiz->izq->izq, termino, mostrar_traza);
            if (mostrar_traza) {
                printf("  [Splay] Rotacion Zig-Zig (derecha doble) sobre '%s'\n",
                       raiz->termino_buscado);
            }
            raiz = rotar_derecha(raiz);
        } else if (cmp_izq > 0) {
            // caso zig-zag: primero izquierda y luego derecha
            raiz->izq->der = splay_recursivo(raiz->izq->der, termino, mostrar_traza);
            if (raiz->izq->der != NULL) {
                if (mostrar_traza) {
                    printf("  [Splay] Rotacion Zig-Zag (izq en hijo '%s', der en '%s')\n",
                           raiz->izq->termino_buscado, raiz->termino_buscado);
                }
                raiz->izq = rotar_izquierda(raiz->izq);
            }
        }

        // realiza la rotacion final hacia la derecha
        if (!raiz->izq) return raiz;
        if (cmp_izq == 0 && mostrar_traza) {
            printf("  [Splay] Rotacion Zig (derecha simple) sobre '%s'\n",
                   raiz->termino_buscado);
        }
        return rotar_derecha(raiz);
    }
    // busca en el subarbol derecho
    else {
        if (!raiz->der) return raiz;

        int cmp_der = strcmp(termino, raiz->der->termino_buscado);

        if (cmp_der < 0) {
            // caso zag-zig: primero derecha y luego izquierda
            raiz->der->izq = splay_recursivo(raiz->der->izq, termino, mostrar_traza);
            if (raiz->der->izq != NULL) {
                if (mostrar_traza) {
                    printf("  [Splay] Rotacion Zag-Zig (der en hijo '%s', izq en '%s')\n",
                           raiz->der->termino_buscado, raiz->termino_buscado);
                }
                raiz->der = rotar_derecha(raiz->der);
            }
        } else if (cmp_der > 0) {
            // caso zag-zag: dos rotaciones hacia la izquierda
            raiz->der->der = splay_recursivo(raiz->der->der, termino, mostrar_traza);
            if (mostrar_traza) {
                printf("  [Splay] Rotacion Zag-Zag (izquierda doble) sobre '%s'\n",
                       raiz->termino_buscado);
            }
            raiz = rotar_izquierda(raiz);
        }

        // realiza la rotacion final hacia la izquierda
        if (!raiz->der) return raiz;
        if (cmp_der == 0 && mostrar_traza) {
            printf("  [Splay] Rotacion Zag (izquierda simple) sobre '%s'\n",
                   raiz->termino_buscado);
        }
        return rotar_izquierda(raiz);
    }
}

// crea un nuevo nodo y guarda los resultados de la busqueda
static NodoSplay *crear_nodo_splay(const char *termino, ListaArchivos *resultados) {
    NodoSplay *nuevo = (NodoSplay *)malloc(sizeof(NodoSplay));
    if (!nuevo) return NULL;

    nuevo->termino_buscado = strdup(termino);
    if (!nuevo->termino_buscado) {
        free(nuevo);
        return NULL;
    }

    if (resultados) {
        nuevo->resultados = *resultados;
        lista_archivos_inicializar(resultados);
    } else {
        lista_archivos_inicializar(&nuevo->resultados);
    }

    nuevo->accesos = 1;
    nuevo->izq = NULL;
    nuevo->der = NULL;
    return nuevo;
}

// muestra los nodos del arbol y sus resultados
static void imprimir_recursivo(const NodoSplay *nodo, int nivel) {
    if (!nodo) return;

    imprimir_recursivo(nodo->der, nivel + 1);

    for (int i = 0; i < nivel; i++) {
        printf("    ");
    }
    printf("|-- \"%s\" (libros: %zu, accesos: %zu)%s\n",
           nodo->termino_buscado,
           nodo->resultados.cantidad,
           nodo->accesos,
           (nivel == 0) ? "  <-- [RAIZ]" : "");

    imprimir_recursivo(nodo->izq, nivel + 1);
}

// libera la memoria de todos los nodos del arbol
static void destruir_recursivo(NodoSplay *nodo) {
    if (!nodo) return;
    destruir_recursivo(nodo->izq);
    destruir_recursivo(nodo->der);
    free(nodo->termino_buscado);
    lista_archivos_liberar(&nodo->resultados);
    free(nodo);
}

// crea un arbol chaflaneado vacio
ArbolChaflaneado *splay_crear(void) {
    ArbolChaflaneado *arbol = (ArbolChaflaneado *)malloc(sizeof(ArbolChaflaneado));
    if (!arbol) return NULL;
    arbol->raiz = NULL;
    arbol->total_nodos = 0;
    return arbol;
}

// busca un termino en el arbol y lo mueve hacia la raiz
NodoSplay *splay_buscar(ArbolChaflaneado *arbol, const char *termino, bool mostrar_traza) {
    if (!arbol || !termino || !arbol->raiz) return NULL;

    if (mostrar_traza) {
        printf("[Splay] Ejecutando chaflaneo para termino: \"%s\"\n", termino);
    }

    arbol->raiz = splay_recursivo(arbol->raiz, termino, mostrar_traza);

    if (arbol->raiz && strcmp(arbol->raiz->termino_buscado, termino) == 0) {
        arbol->raiz->accesos++;
        return arbol->raiz;
    }

    return NULL;
}

// inserta un termino nuevo y lo coloca en la raiz
NodoSplay *splay_insertar(ArbolChaflaneado *arbol,
                          const char *termino,
                          ListaArchivos *resultados,
                          bool mostrar_traza) {
    if (!arbol || !termino) return NULL;

    // si el arbol esta vacio, el nuevo nodo se convierte en la raiz
    if (!arbol->raiz) {
        NodoSplay *nuevo = crear_nodo_splay(termino, resultados);
        if (nuevo) {
            arbol->raiz = nuevo;
            arbol->total_nodos++;
        }
        return nuevo;
    }

    // mueve el termino mas cercano a la raiz antes de insertar
    arbol->raiz = splay_recursivo(arbol->raiz, termino, mostrar_traza);

    int cmp = strcmp(termino, arbol->raiz->termino_buscado);
    if (cmp == 0) {
        // si el termino ya existe, actualiza sus resultados
        arbol->raiz->accesos++;
        if (resultados) {
            lista_archivos_liberar(&arbol->raiz->resultados);
            arbol->raiz->resultados = *resultados;
            lista_archivos_inicializar(resultados);
        }
        return arbol->raiz;
    }

    NodoSplay *nuevo = crear_nodo_splay(termino, resultados);
    if (!nuevo) return NULL;

    if (cmp < 0) {
        nuevo->der = arbol->raiz;
        nuevo->izq = arbol->raiz->izq;
        arbol->raiz->izq = NULL;
    } else {
        nuevo->izq = arbol->raiz;
        nuevo->der = arbol->raiz->der;
        arbol->raiz->der = NULL;
    }

    arbol->raiz = nuevo;
    arbol->total_nodos++;
    return nuevo;
}

// muestra el estado actual del arbol chaflaneado
void splay_imprimir_arbol(const ArbolChaflaneado *arbol) {
    printf("\n=== Estado del Arbol Chaflaneado (Cache de Busquedas) ===\n");
    if (!arbol || !arbol->raiz) {
        printf("(Arbol vacio)\n");
        printf("=========================================================\n");
        return;
    }
    printf("Total de terminos en cache: %zu\n\n", arbol->total_nodos);
    imprimir_recursivo(arbol->raiz, 0);
    printf("=========================================================\n");
}

// libera toda la memoria utilizada por el arbol
void splay_destruir(ArbolChaflaneado *arbol) {
    if (!arbol) return;
    destruir_recursivo(arbol->raiz);
    free(arbol);
}