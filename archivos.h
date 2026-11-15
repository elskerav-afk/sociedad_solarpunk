#ifndef ARCHIVOS_H
#define ARCHIVOS_H

#include <stdbool.h>
#include <stddef.h>

// funcion que se ejecuta por cada archivo .txt encontrado
// recibe el nombre del archivo, su ruta y un contexto
typedef void (*CallbackArchivoTxt)(const char *nombre_archivo,
                                   const char *ruta_completa,
                                   void *contexto);

// recorre una carpeta y sus subcarpetas buscando archivos .txt
// ejecuta el callback por cada archivo encontrado
// retorna la cantidad de archivos encontrados o -1 si ocurre un error
int archivos_recorrer_directorio(const char *ruta_directorio,
                                 CallbackArchivoTxt callback,
                                 void *contexto);

// busca un texto dentro de un archivo leyendo linea por linea
// permite ignorar las mayusculas y minusculas
// retorna true si encuentra el texto o false si no lo encuentra
bool archivos_contiene_texto(const char *ruta_archivo,
                           const char *texto_buscado,
                           bool ignorar_mayusculas);

// muestra las primeras lineas de un archivo
// retorna true si pudo leerlo o false si ocurre un error
bool archivos_mostrar_fragmento(const char *ruta_archivo, size_t max_lineas);

#endif /* ARCHIVOS_H */