#define _POSIX_C_SOURCE 200809L
#include "archivos.h"

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define BUFFER_LINEA 4096
#define MAX_RUTA 1024

// verifica si el archivo termina en .txt
static bool es_archivo_txt(const char *nombre) {
    if (!nombre) return false;
    size_t len = strlen(nombre);
    if (len < 4) return false;

    const char *ext = nombre + (len - 4);
    return (tolower((unsigned char)ext[0]) == '.' &&
            tolower((unsigned char)ext[1]) == 't' &&
            tolower((unsigned char)ext[2]) == 'x' &&
            tolower((unsigned char)ext[3]) == 't');
}

// busca un texto dentro de otro ignorando mayusculas y minusculas
static const char *strstr_insensible(const char *pajar, const char *aguja) {
    if (!*aguja) return pajar;

    for (; *pajar; pajar++) {
        if (tolower((unsigned char)*pajar) == tolower((unsigned char)*aguja)) {
            const char *h = pajar + 1;
            const char *n = aguja + 1;
            while (*h && *n &&
                   tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
                h++;
                n++;
            }
            if (!*n) return pajar;
        }
    }
    return NULL;
}

int archivos_recorrer_directorio(const char *ruta_directorio,
                                 CallbackArchivoTxt callback,
                                 void *contexto) {
    if (!ruta_directorio || !callback) return -1;

    DIR *dir = opendir(ruta_directorio);
    if (!dir) {
        perror("Error al abrir directorio de biblioteca");
        return -1;
    }

    int total_encontrados = 0;
    struct dirent *entrada;
    char ruta_completa[MAX_RUTA];

    while ((entrada = readdir(dir)) != NULL) {
        // ignora la carpeta actual y la carpeta anterior
        if (strcmp(entrada->d_name, ".") == 0 ||
            strcmp(entrada->d_name, "..") == 0) {
            continue;
        }

        // construye la ruta completa del archivo o carpeta
        size_t len_dir = strlen(ruta_directorio);
        int escritos;
        if (len_dir > 0 && ruta_directorio[len_dir - 1] == '/') {
            escritos = snprintf(ruta_completa, sizeof(ruta_completa), "%s%s",
                                ruta_directorio, entrada->d_name);
        } else {
            escritos = snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s",
                                ruta_directorio, entrada->d_name);
        }

        // verifica que la ruta no supere el tamaño del buffer
        if (escritos < 0 || (size_t)escritos >= sizeof(ruta_completa)) {
            fprintf(stderr, "Advertencia: ruta demasiado larga omitida: %s/%s\n",
                    ruta_directorio, entrada->d_name);
            continue;
        }

        struct stat info;
        if (stat(ruta_completa, &info) != 0) {
            continue;
        }

        // si encuentra una carpeta, la recorre tambien
        if (S_ISDIR(info.st_mode)) {
            int sub_total = archivos_recorrer_directorio(ruta_completa, callback, contexto);
            if (sub_total > 0) {
                total_encontrados += sub_total;
            }
        // si es un archivo .txt, ejecuta el callback
        } else if (S_ISREG(info.st_mode) && es_archivo_txt(entrada->d_name)) {
            callback(entrada->d_name, ruta_completa, contexto);
            total_encontrados++;
        }
    }

    closedir(dir);
    return total_encontrados;
}

bool archivos_contiene_texto(const char *ruta_archivo,
                           const char *texto_buscado,
                           bool ignorar_mayusculas) {
    if (!ruta_archivo || !texto_buscado || texto_buscado[0] == '\0') {
        return false;
    }

    FILE *archivo = fopen(ruta_archivo, "r");
    if (!archivo) {
        return false;
    }

    char linea[BUFFER_LINEA];
    bool encontrado = false;

    // lee el archivo linea por linea hasta encontrar el texto
    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        const char *match = ignorar_mayusculas
                                ? strstr_insensible(linea, texto_buscado)
                                : strstr(linea, texto_buscado);
        if (match != NULL) {
            encontrado = true;
            break; // ya encontro el texto, no necesita seguir leyendo
        }
    }

    fclose(archivo);
    return encontrado;
}

bool archivos_mostrar_fragmento(const char *ruta_archivo, size_t max_lineas) {
    if (!ruta_archivo || max_lineas == 0) return false;

    FILE *archivo = fopen(ruta_archivo, "r");
    if (!archivo) {
        perror("No se pudo abrir el libro");
        return false;
    }

    char linea[BUFFER_LINEA];
    size_t contador = 0;

    // muestra las primeras lineas del archivo
    printf("\n--- Inicio de extracto (%s) ---\n", ruta_archivo);
    while (contador < max_lineas && fgets(linea, sizeof(linea), archivo) != NULL) {
        printf("%s", linea);
        contador++;
    }
    printf("\n--- Fin de extracto ---\n");

    fclose(archivo);
    return true;
}