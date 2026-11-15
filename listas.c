
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "listas.h"

// deja la lista vacía y pone la cantidad de nodos en cero
void inicializar_lista(Lista *lista) {
    lista->inicio = NULL;
    lista->final = NULL;
    lista->cantidad = 0;
}

// crea un nodo con el nombre y lo agrega al final de la lista
int insertar_final(Lista *lista, const char *nombre) {
    Nodo *nuevo = malloc(sizeof(Nodo));

    if (nuevo == NULL) {
        return 0;
    }

    nuevo->nombre = malloc(strlen(nombre) + 1);

    if (nuevo->nombre == NULL) {
        free(nuevo);
        return 0;
    }

    strcpy(nuevo->nombre, nombre);
    nuevo->siguiente = NULL;

    if (lista->inicio == NULL) {
        lista->inicio = nuevo;
    } else {
        lista->final->siguiente = nuevo;
    }

    lista->final = nuevo;
    lista->cantidad++;

    return 1;
}

// recorre los nodos desde el inicio e imprime cada nombre
void imprimir_lista(const Lista *lista) {
    Nodo *actual = lista->inicio;

    while (actual != NULL) {
        printf("%s\n", actual->nombre);
        actual = actual->siguiente;
    }
}

// recorre la lista, libera la memoria y la deja vacía
void liberar_lista(Lista *lista) {
    Nodo *actual = lista->inicio;

    while (actual != NULL) {
        Nodo *siguiente = actual->siguiente;

        free(actual->nombre);
        free(actual);

        actual = siguiente;
    }

    inicializar_lista(lista);
}

// lee un archivo y guarda sus nombres en una lista
int cargar_archivo(const char *nombre_archivo, Lista *lista) {
    FILE *archivo = fopen(nombre_archivo, "r");

    // revisa si el archivo se pudo abrir
    if (archivo == NULL) {
        printf("no se pudo abrir el archivo: %s\n", nombre_archivo);
        return 0;
    }

    char *linea = NULL;
    size_t capacidad = 0;
    ssize_t longitud;
    int resultado = 1;

    // lee el archivo linea por linea
    while ((longitud = getline(&linea, &capacidad, archivo)) != -1) {

        // elimina el salto de linea del texto
        linea[strcspn(linea, "\r\n")] = '\0';

        // guarda el nombre si la linea no esta vacia
        if (strlen(linea) > 0) {
            if (!insertar_final(lista, linea)) {
                printf("no se pudo guardar un nombre en la lista\n");
                resultado = 0;
                break;
            }
        }
    }

    // libera la memoria usada para leer las lineas
    free(linea);

    // cierra el archivo
    fclose(archivo);

    return resultado;
}
