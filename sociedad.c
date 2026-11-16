
#include "sociedad.h"
#include "listas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// hace una copia del nombre para guardarlo en memoria
char *copiar_texto(const char *texto) {
    char *copia = malloc(strlen(texto) + 1);

    if (copia == NULL) {
        return NULL;
    }

    strcpy(copia, texto);
    return copia;
}

// agrega una persona a la lista
int agregar_persona(struct Sociedad *sociedad, const char *nombre) {
    struct Persona *nueva = malloc(sizeof(struct Persona));

    if (nueva == NULL) {
        return 0;
    }

    nueva->nombre = copiar_texto(nombre);

    if (nueva->nombre == NULL) {
        free(nueva);
        return 0;
    }

    nueva->comunaActual = NULL;
    nueva->librosLeidos = NULL;
    nueva->servicios = NULL;
    nueva->regalosDados = 0;
    nueva->regalosRecibidos = 0;
    nueva->siguiente = NULL;

    if (sociedad->personas == NULL) {
        sociedad->personas = nueva;
    } else {
        struct Persona *actual = sociedad->personas;

        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }

        actual->siguiente = nueva;
    }

    return 1;
}

int persona_aprender_servicio(struct Persona *p, const char *servicio) {
    struct ServicioPersona *actual = p->servicios;
    struct ServicioPersona *nuevo;

    // si ya lo sabe, no lo repetimos
    while (actual != NULL) {
        if (strcmp(actual->nombre, servicio) == 0) {
            return 0;
        }
        actual = actual->siguiente;
    }

    nuevo = malloc(sizeof(struct ServicioPersona));

    if (nuevo == NULL) {
        return 0;
    }

    nuevo->nombre = copiar_texto(servicio);

    if (nuevo->nombre == NULL) {
        free(nuevo);
        return 0;
    }

    // se inserta al inicio, es lo mas simple
    nuevo->siguiente = p->servicios;
    p->servicios = nuevo;
    return 1;
}

int registrar_libro_leido(struct Persona *p, const char *nombreArchivo) {
    struct LibroLeido *actual = p->librosLeidos;
    struct LibroLeido *nuevo;

    // si ya lo leyo, no se repite
    while (actual != NULL) {
        if (strcmp(actual->nombreArchivo, nombreArchivo) == 0) {
            return 0;
        }
        actual = actual->siguiente;
    }

    nuevo = malloc(sizeof(struct LibroLeido));

    if (nuevo == NULL) {
        return 0;
    }

    nuevo->nombreArchivo = copiar_texto(nombreArchivo);

    if (nuevo->nombreArchivo == NULL) {
        free(nuevo);
        return 0;
    }

    nuevo->siguiente = p->librosLeidos;
    p->librosLeidos = nuevo;
    return 1;
}

int contar_libros_leidos(const struct Persona *p) {
    const struct LibroLeido *libro = p->librosLeidos;
    int total = 0;

    while (libro != NULL) {
        total++;
        libro = libro->siguiente;
    }

    return total;
}

int aprender_servicio_leyendo(struct Persona *p, const char *servicio) {
    struct Existencia *e;
    struct LibroLeido *libro;

    if (contar_libros_leidos(p) < 3) {
        return 0;
    }

    // devuelve 0 si ya lo sabia
    if (!persona_aprender_servicio(p, servicio)) {
        return 0;
    }

    // el stock de ese servicio crece en su comuna
    e = p->comunaActual->servicios;

    while (e != NULL) {
        if (strcmp(e->nombre, servicio) == 0) {
            e->maximo += 2;
            e->cantidad += 2;
        }
        e = e->siguiente;
    }

    // gasta los libros: para otro servicio necesita leer 3 nuevos
    while (p->librosLeidos != NULL) {
        libro = p->librosLeidos;
        p->librosLeidos = libro->siguiente;
        free(libro->nombreArchivo);
        free(libro);
    }

    return 1;
}

// agrega un bien al catalogo de bienes
int agregar_bien(struct Sociedad *sociedad, const char *nombre) {
    struct NodoNombreDoble *nuevo = malloc(sizeof(struct NodoNombreDoble));

    if (nuevo == NULL) {
        return 0;
    }

    nuevo->nombre = copiar_texto(nombre);

    if (nuevo->nombre == NULL) {
        free(nuevo);
        return 0;
    }

    nuevo->anterior = NULL;
    nuevo->siguiente = NULL;

    if (sociedad->catalogoBienes == NULL) {
        sociedad->catalogoBienes = nuevo;
    } else {
        struct NodoNombreDoble *actual = sociedad->catalogoBienes;

        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }

        actual->siguiente = nuevo;
        nuevo->anterior = actual;
    }

    return 1;
}

// agrega un servicio al catalogo de servicios
int agregar_servicio(struct Sociedad *sociedad, const char *nombre) {
    struct NodoNombre *nuevo = malloc(sizeof(struct NodoNombre));

    if (nuevo == NULL) {
        return 0;
    }

    nuevo->nombre = copiar_texto(nombre);

    if (nuevo->nombre == NULL) {
        free(nuevo);
        return 0;
    }

    nuevo->siguiente = NULL;

    if (sociedad->catalogoServicios == NULL) {
        sociedad->catalogoServicios = nuevo;
    } else {
        struct NodoNombre *actual = sociedad->catalogoServicios;

        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }

        actual->siguiente = nuevo;
    }

    return 1;
}

// calcula la reciprocidad de una comuna: 100 = dados y recibidos equilibrados
void calcular_reciprocidad_comuna(struct Sociedad *sociedad,
                                  struct Comuna *comuna) {
    int dados = 0;
    int recibidos = 0;
    struct Persona *p = sociedad->personas;

    while (p != NULL) {
        if (p->comunaActual == comuna) {
            dados += p->regalosDados;
            recibidos += p->regalosRecibidos;
        }
        p = p->siguiente;
    }

        int diferencia = dados - recibidos;
    int limite = comuna->poblacion / 10;

    if (diferencia < 0) {
        diferencia = -diferencia;
    }

    if (limite < 1) {
        limite = 1;
    }

    comuna->reciprocidad = 100.0f - 100.0f * diferencia / limite;

    if (comuna->reciprocidad < 0) {
        comuna->reciprocidad = 0;
    }
}


 // agrega una comuna y guarda su nombre en la lista doble
int agregar_comuna(struct Sociedad *sociedad, const char *nombre) {
    struct Comuna *nueva = malloc(sizeof(struct Comuna));
    struct NodoNombreDoble *nuevoNombre;

    if (nueva == NULL) {
        return 0;
    }

    nueva->nombre = copiar_texto(nombre);

    if (nueva->nombre == NULL) {
        free(nueva);
        return 0;
    }

    // inicializa los datos de la comuna
    nueva->poblacion = 0;
    nueva->bienes = NULL;
    nueva->servicios = NULL;
    nueva->necesidad = 0;
    nueva->satisfaccion = 0;
    nueva->penalizacionEmergencia = 0;
    nueva->reciprocidad = 100;
    nueva->administrada = 0;
    nueva->anterior = NULL;
    nueva->siguiente = NULL;
    nueva->biblioteca = NULL;
    
    // reserva memoria para guardar tambien el nombre
    nuevoNombre = malloc(sizeof(struct NodoNombreDoble));

    if (nuevoNombre == NULL) {
        free(nueva->nombre);
        free(nueva);
        return 0;
    }

    nuevoNombre->nombre = copiar_texto(nombre);

    if (nuevoNombre->nombre == NULL) {
        free(nuevoNombre);
        free(nueva->nombre);
        free(nueva);
        return 0;
    }

    nuevoNombre->anterior = NULL;
    nuevoNombre->siguiente = NULL;

    // agrega la comuna a la lista principal
    if (sociedad->comunas == NULL) {
        sociedad->comunas = nueva;
    } else {
        struct Comuna *actual = sociedad->comunas;

        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }

        actual->siguiente = nueva;
        nueva->anterior = actual;
    }

    // agrega el nombre a la lista doble
    if (sociedad->nombresComunas == NULL) {
        sociedad->nombresComunas = nuevoNombre;
    } else {
        struct NodoNombreDoble *actual = sociedad->nombresComunas;

        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }

        actual->siguiente = nuevoNombre;
        nuevoNombre->anterior = actual;
    }

    return 1;
}



// agrega un bien o servicio a las existencias de una comuna
int agregar_existencia(struct Existencia **lista, const char *nombre,
                       int maximo) {
    struct Existencia *nuevo = malloc(sizeof(struct Existencia));

    if (nuevo == NULL) {
        return 0;
    }

    nuevo->nombre = copiar_texto(nombre);

    if (nuevo->nombre == NULL) {
        free(nuevo);
        return 0;
    }

    nuevo->maximo = maximo;
    nuevo->cantidad = maximo * 4 / 5;
    nuevo->valor = 1.0f;
    nuevo->siguiente = NULL;

    if (*lista == NULL) {
        *lista = nuevo;
    } else {
        struct Existencia *actual = *lista;

        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }

        actual->siguiente = nuevo;
    }

    return 1;
}

// crea las existencias de una comuna
int crear_existencias_comuna(struct Sociedad *sociedad,
                             struct Comuna *comuna) {
    struct NodoNombreDoble *bien = sociedad->catalogoBienes;
    struct NodoNombre *servicio = sociedad->catalogoServicios;
    int i;

    for (i = 0; i < sociedad->cantidadBienesActivos; i++) {
        if (bien == NULL) {
            return 0;
        }

        if (!agregar_existencia(&comuna->bienes, bien->nombre,
                                comuna->poblacion * 5)) {
            return 0;
        }

        bien = bien->siguiente;
    }

    for (i = 0; i < sociedad->cantidadServiciosActivos; i++) {
        if (servicio == NULL) {
            return 0;
        }

        if (!agregar_existencia(&comuna->servicios, servicio->nombre,
                                comuna->poblacion * 2)) {
            return 0;
        }

        servicio = servicio->siguiente;
    }

    return 1;
}

// calcula la necesidad y la satisfaccion de una comuna
void calcular_indices_comuna(struct Comuna *comuna) {
    struct Existencia *actual;
    int maximoTotal = 0;
    int cantidadTotal = 0;

    if (comuna == NULL) {
        return;
    }

    actual = comuna->bienes;
    while (actual != NULL) {
        maximoTotal += actual->maximo;
        cantidadTotal += actual->cantidad;
        actual = actual->siguiente;
    }

    actual = comuna->servicios;
    while (actual != NULL) {
        maximoTotal += actual->maximo;
        cantidadTotal += actual->cantidad;
        actual = actual->siguiente;
    }

    if (maximoTotal > 0) {
        comuna->necesidad =
            100.0f * (maximoTotal - cantidadTotal) / maximoTotal;
    } else {
        comuna->necesidad = 0;
    }

    if (comuna->necesidad < 0) {
        comuna->necesidad = 0;
    }

    if (comuna->necesidad >= 100) {
        comuna->necesidad = 99.9f;
    }

    comuna->satisfaccion =
    100.0f - comuna->necesidad
    - comuna->penalizacionEmergencia;

    if (comuna->satisfaccion < 0) {
        comuna->satisfaccion = 0;
    }
}


// calcula los indices de todas las comunas y mezcla la satisfaccion con la de las cercanas
void calcular_indices_sociedad(struct Sociedad *sociedad) {
    struct Comuna *actual;
    struct Comuna *vecina;
    float bienestarBase[50];
    float necesidadBase[50];
    int cantidad = 0;

    if (sociedad == NULL) return;

    actual = sociedad->comunas;
    while (actual != NULL && cantidad < 50) {
        calcular_indices_comuna(actual);
        necesidadBase[cantidad] = actual->necesidad;
        bienestarBase[cantidad] = 100.0f - actual->necesidad - actual->penalizacionEmergencia;

        if (bienestarBase[cantidad] < 0) bienestarBase[cantidad] = 0;
        if (bienestarBase[cantidad] > 100) bienestarBase[cantidad] = 100;

        actual = actual->siguiente;
        cantidad++;
    }

    actual = sociedad->comunas;
    for (int i = 0; actual != NULL && i < cantidad; i++) {
        float sumaSatVecinas = 0.0f;
        float sumaNecVecinas = 0.0f;
        int cantidadVecinas = 0;
        int regalosRecibidosComuna = 0;

        calcular_reciprocidad_comuna(sociedad, actual);

        // suma regalos recibidos por los habitantes de esta comuna
        struct Persona *p = sociedad->personas;
        while (p != NULL) {
            if (p->comunaActual == actual) {
                regalosRecibidosComuna += p->regalosRecibidos;
            }
            p = p->siguiente;
        }

        vecina = sociedad->comunas;
        for (int j = 0; vecina != NULL && j < cantidad; j++) {
            int distancia = (i > j) ? (i - j) : (j - i);
            if (distancia >= 1 && distancia <= 2) {
                sumaSatVecinas += bienestarBase[j];
                sumaNecVecinas += necesidadBase[j];
                cantidadVecinas++;
            }
            vecina = vecina->siguiente;
        }

        float bonoRegalos = (regalosRecibidosComuna > 20) ? 5.0f : (regalosRecibidosComuna * 0.25f);

        if (cantidadVecinas > 0) {
            float promSatVecinas = sumaSatVecinas / cantidadVecinas;
            float promNecVecinas = sumaNecVecinas / cantidadVecinas;

            // incorpora necesidad local, necesidad de circundantes, satisfaccion de circundantes, reciprocidad y regalos recibidos
            actual->satisfaccion =
                0.50f * bienestarBase[i] +
                0.20f * promSatVecinas +
                0.15f * actual->reciprocidad -
                0.10f * promNecVecinas +
                bonoRegalos;
        } else {
            actual->satisfaccion =
                0.80f * bienestarBase[i] +
                0.15f * actual->reciprocidad +
                bonoRegalos;
        }

        if (actual->satisfaccion < 0) actual->satisfaccion = 0;
        if (actual->satisfaccion > 100) actual->satisfaccion = 100;

        actual = actual->siguiente;
    }
}

// carga los datos iniciales de la sociedad
int inicializar_sociedad(struct Sociedad *sociedad,
                         const char *archivoPersonas,
                         const char *archivoComunas,
                         const char *archivoBienes,
                         const char *archivoServicios) {

    // deja la sociedad vacia antes de empezar
    sociedad->poblacionTotal = 0;
    sociedad->cantidadComunas = 0;
    sociedad->cantidadBienesActivos = 0;
    sociedad->cantidadServiciosActivos = 0;
    sociedad->dia = 0;
    sociedad->diasParaEmergencia = (rand() % 5) + 3;

    sociedad->personas = NULL;
    sociedad->catalogoServicios = NULL;
    sociedad->catalogoBienes = NULL;
    sociedad->nombresComunas = NULL;
    sociedad->comunas = NULL;

    // listas temporales para leer los archivos
    Lista personas;
    Lista comunas;
    Lista bienes;
    Lista servicios;

    inicializar_lista(&personas);
    inicializar_lista(&comunas);
    inicializar_lista(&bienes);
    inicializar_lista(&servicios);

    // lee los cuatro archivos
    if (!cargar_archivo(archivoPersonas, &personas) ||
        !cargar_archivo(archivoComunas, &comunas) ||
        !cargar_archivo(archivoBienes, &bienes) ||
        !cargar_archivo(archivoServicios, &servicios)) {

        liberar_lista(&personas);
        liberar_lista(&comunas);
        liberar_lista(&bienes);
        liberar_lista(&servicios);
        return 0;
    }

    // copia los nombres de las personas
    Nodo *actual = personas.inicio;

    while (actual != NULL) {
        if (!agregar_persona(sociedad, actual->nombre)) {
            liberar_lista(&personas);
            liberar_lista(&comunas);
            liberar_lista(&bienes);
            liberar_lista(&servicios);
            liberar_sociedad(sociedad);
            return 0;
        }

        sociedad->poblacionTotal++;
        actual = actual->siguiente;
    }

    // crea las comunas
    
    int comunasActivas;
    int bienesActivos;
    int serviciosActivos;

    printf("Cantidad de comunas activas (3-%d): ",
           comunas.cantidad);

    if (scanf("%d", &comunasActivas) != 1 ||
        comunasActivas < 3 ||
        comunasActivas > comunas.cantidad) {
        printf("Cantidad de comunas no valida\n");
        liberar_lista(&personas);
        liberar_lista(&comunas);
        liberar_lista(&bienes);
        liberar_lista(&servicios);
        liberar_sociedad(sociedad);
        return 0;
    }

    printf("Cantidad de bienes activos (1-%d): ",
           bienes.cantidad);

    if (scanf("%d", &bienesActivos) != 1 ||
        bienesActivos < 1 ||
        bienesActivos > bienes.cantidad) {
        printf("Cantidad de bienes no valida\n");
        liberar_lista(&personas);
        liberar_lista(&comunas);
        liberar_lista(&bienes);
        liberar_lista(&servicios);
        liberar_sociedad(sociedad);
        return 0;
    }

    printf("Cantidad de servicios activos (1-%d): ",
           servicios.cantidad);

    if (scanf("%d", &serviciosActivos) != 1 ||
        serviciosActivos < 1 ||
        serviciosActivos > servicios.cantidad) {
        printf("Cantidad de servicios no valida\n");
        liberar_lista(&personas);
        liberar_lista(&comunas);
        liberar_lista(&bienes);
        liberar_lista(&servicios);
        liberar_sociedad(sociedad);
        return 0;
    }

    sociedad->cantidadBienesActivos = bienesActivos;
    sociedad->cantidadServiciosActivos = serviciosActivos;

    // copia los bienes del catalogo
    actual = bienes.inicio;

    while (actual != NULL) {
        if (!agregar_bien(sociedad, actual->nombre)) {
            liberar_lista(&personas);
            liberar_lista(&comunas);
            liberar_lista(&bienes);
            liberar_lista(&servicios);
            liberar_sociedad(sociedad);
            return 0;
        }

        actual = actual->siguiente;
    }

    // copia los servicios del catalogo
    actual = servicios.inicio;

    while (actual != NULL) {
        if (!agregar_servicio(sociedad, actual->nombre)) {
            liberar_lista(&personas);
            liberar_lista(&comunas);
            liberar_lista(&bienes);
            liberar_lista(&servicios);
            liberar_sociedad(sociedad);
            return 0;
        }

        actual = actual->siguiente;
    }

    // crea solamente las comunas seleccionadas
    actual = comunas.inicio;

    while (actual != NULL && sociedad->cantidadComunas <
                              comunasActivas) {
        if (!agregar_comuna(sociedad, actual->nombre)) {
            liberar_lista(&personas);
            liberar_lista(&comunas);
            liberar_lista(&bienes);
            liberar_lista(&servicios);
            liberar_sociedad(sociedad);
            return 0;
        }

        sociedad->cantidadComunas++;
        actual = actual->siguiente;
    }

    // reparte la poblacion entre las comunas
    struct Comuna *comunaActual = sociedad->comunas;
    int poblacionBase = sociedad->poblacionTotal / comunasActivas;
    int personasExtra = sociedad->poblacionTotal % comunasActivas;
    int numeroComuna = 0;
    struct Persona *personaAsignar = sociedad->personas;

    while (comunaActual != NULL) {
        comunaActual->poblacion = poblacionBase;
        comunaActual->administrada = (numeroComuna < 3);

        if (numeroComuna < personasExtra) {
            comunaActual->poblacion++;
        }
        
        // asigna a esta comuna las siguientes personas de la lista
		for (int i = 0; i < comunaActual->poblacion; i++) {
		personaAsignar->comunaActual = comunaActual;
		personaAsignar = personaAsignar->siguiente;
		}

        if (!crear_existencias_comuna(sociedad, comunaActual)) {
            liberar_lista(&personas);
            liberar_lista(&comunas);
            liberar_lista(&bienes);
            liberar_lista(&servicios);
            liberar_sociedad(sociedad);
            return 0;
        }

        calcular_indices_comuna(comunaActual);

        numeroComuna++;
        comunaActual = comunaActual->siguiente;
    }


    // libera las listas temporales porque ya copiamos los datos
    liberar_lista(&personas);
    liberar_lista(&comunas);
    liberar_lista(&bienes);
    liberar_lista(&servicios);
	// asigna servicios iniciales a las personas (0, 1 o 2)
	struct Persona *personaServ = sociedad->personas;

	while (personaServ != NULL) {
		int cantidadServicios = rand() % 3;

		for (int k = 0; k < cantidadServicios; k++) {
			struct NodoNombre *servicio = sociedad->catalogoServicios;
			int posicion = rand() % sociedad->cantidadServiciosActivos;

			for (int j = 0; j < posicion; j++) {
				servicio = servicio->siguiente;
			}

			persona_aprender_servicio(personaServ, servicio->nombre);
		}	

		personaServ = personaServ->siguiente;
	}
    return 1;
}

// construye el AVL compartido y crea la biblioteca de cada comuna
int crear_bibliotecas(struct Sociedad *sociedad, const char *rutaCorpus) {
    struct Comuna *comuna;

    sociedad->indiceLibros = biblioteca_construir_indice_avl(rutaCorpus);

    if (sociedad->indiceLibros == NULL) {
        return 0;
    }

    comuna = sociedad->comunas;

    while (comuna != NULL) {
        comuna->biblioteca = biblioteca_crear_con_indice(
            comuna->nombre, sociedad->indiceLibros);

        if (comuna->biblioteca == NULL) {
            return 0;
        }

        comuna = comuna->siguiente;
    }

    return 1;
}

// libera una lista de existencias
void liberar_existencias(struct Existencia *lista) {
    struct Existencia *actual;

    while (lista != NULL) {
        actual = lista;
        lista = lista->siguiente;

        free(actual->nombre);
        free(actual);
    }
}
// disuelve una comuna: sus personas se mudan a la comuna con mas satisfaccion
// los servicios viajan con las personas y los bienes se pierden
int disolver_comuna(struct Sociedad *sociedad, struct Comuna *comuna) {
    struct Comuna *destino = NULL;
    struct Comuna *actual = sociedad->comunas;
    struct Persona *p;

    // busca la comuna destino: la de mayor satisfaccion que no sea la disuelta
    while (actual != NULL) {
        if (actual != comuna &&
            (destino == NULL || actual->satisfaccion > destino->satisfaccion)) {
            destino = actual;
        }
        actual = actual->siguiente;
    }

    if (destino == NULL) {
        return 0;
    }

    printf("La comuna %s se disuelve. Sus personas se mudan a %s.\n",
           comuna->nombre, destino->nombre);

    // muda a las personas de la comuna
    p = sociedad->personas;

    while (p != NULL) {
        if (p->comunaActual == comuna) {
            struct ServicioPersona *s = p->servicios;

            p->comunaActual = destino;
            destino->poblacion++;

            // sus servicios viajan con ella: sube el stock del destino
            while (s != NULL) {
                struct Existencia *e = destino->servicios;

                while (e != NULL) {
                    if (strcmp(e->nombre, s->nombre) == 0) {
                        e->maximo += 2;
                        e->cantidad += 2;
                    }
                    e = e->siguiente;
                }
                s = s->siguiente;
            }
        }
        p = p->siguiente;
    }

	// si era administrada, una autonoma ocupa su lugar
	if (comuna->administrada) {
		actual = sociedad->comunas;

		while (actual != NULL) {
			if (actual != comuna && actual->administrada == 0) {
				actual->administrada = 1;
				printf("La comuna %s pasa a ser administrada.\n",
					actual->nombre);
				break;
			}
			actual = actual->siguiente;
		}
}

    // saca la comuna de la lista
    if (comuna->anterior != NULL) {
        comuna->anterior->siguiente = comuna->siguiente;
    } else {
        sociedad->comunas = comuna->siguiente;
    }

    if (comuna->siguiente != NULL) {
        comuna->siguiente->anterior = comuna->anterior;
    }
	// la caché y la biblioteca de la comuna se pierden con ella
	if (comuna->biblioteca != NULL) {
		biblioteca_destruir(comuna->biblioteca);
	}
    // los bienes se pierden y se libera la comuna
    liberar_existencias(comuna->bienes);
    liberar_existencias(comuna->servicios);
    free(comuna->nombre);
    free(comuna);
    sociedad->cantidadComunas--;

    return 1;
}

// disuelve las comunas con necesidad muy alta o satisfaccion muy baja
void revisar_disoluciones(struct Sociedad *sociedad) {
    struct Comuna *actual = sociedad->comunas;
    struct Comuna *siguiente;

    while (actual != NULL) {
        // se guarda antes porque "actual" puede liberarse
        siguiente = actual->siguiente;

        if (actual->necesidad >= UMBRAL_NECESIDAD ||
            actual->satisfaccion <= UMBRAL_SATISFACCION) {
            disolver_comuna(sociedad, actual);
        }

        actual = siguiente;
    }
}
// libera toda la memoria de la sociedad
void liberar_sociedad(struct Sociedad *sociedad) {
    struct NodoNombre *persona;
    struct NodoNombreDoble *bien;
    struct NodoNombreDoble *nombreComuna;
    struct Comuna *comuna;
    struct Persona *personaActual;

    while (sociedad->personas != NULL) {
        personaActual = sociedad->personas;
        sociedad->personas = personaActual->siguiente;
		
		while (personaActual->servicios != NULL) {
			struct ServicioPersona *serv = personaActual->servicios;
			personaActual->servicios = serv->siguiente;
			free(serv->nombre);
			free(serv);
		}
		while (personaActual->librosLeidos != NULL) {
			struct LibroLeido *libro = personaActual->librosLeidos;
			personaActual->librosLeidos = libro->siguiente;
			free(libro->nombreArchivo);
			free(libro);
}
        free(personaActual->nombre);
        free(personaActual);
    }

    while (sociedad->catalogoServicios != NULL) {
        persona = sociedad->catalogoServicios;
        sociedad->catalogoServicios = persona->siguiente;

        free(persona->nombre);
        free(persona);
    }

    while (sociedad->catalogoBienes != NULL) {
        bien = sociedad->catalogoBienes;
        sociedad->catalogoBienes = bien->siguiente;

        free(bien->nombre);
        free(bien);
    }

    while (sociedad->nombresComunas != NULL) {
        nombreComuna = sociedad->nombresComunas;
        sociedad->nombresComunas = nombreComuna->siguiente;

        free(nombreComuna->nombre);
        free(nombreComuna);
    }

    while (sociedad->comunas != NULL) {
        comuna = sociedad->comunas;
        sociedad->comunas = comuna->siguiente;
        if (comuna->biblioteca != NULL) {
			biblioteca_destruir(comuna->biblioteca);
		}
        liberar_existencias(comuna->bienes);
        liberar_existencias(comuna->servicios);

        free(comuna->nombre);
        free(comuna);
    }
    
    if (sociedad->indiceLibros != NULL) {
		avl_destruir(sociedad->indiceLibros);
		sociedad->indiceLibros = NULL;
	}

    while (sociedad->colaSolicitudes != NULL) {
        struct SolicitudBiblioteca *sol = sociedad->colaSolicitudes;
        sociedad->colaSolicitudes = sol->siguiente;
        free(sol->nombrePersona);
        free(sol->terminoBuscado);
        free(sol);
    }
}


// las comunas autonomas (administrada == 0) gestionan sus propios excedentes diariamente
void simular_comunas_autonomas(struct Sociedad *sociedad) {
    struct Comuna *c = sociedad->comunas;
    printf("\n--- Gestion de Comunas Autonomas ---\n");
    while (c != NULL) {
        if (c->administrada == 0 && c->bienes != NULL) {
            // busca el bien con mayor excedente y lo comparte con una vecina necesitada
            struct Existencia *mejor = c->bienes;
            struct Existencia *aux = c->bienes->siguiente;
            while (aux != NULL) {
                if (aux->cantidad > mejor->cantidad) mejor = aux;
                aux = aux->siguiente;
            }
            struct Comuna *dest = (c->siguiente != NULL) ? c->siguiente : c->anterior;
            if (dest != NULL && mejor->cantidad > 5) {
                struct Existencia *rec = dest->bienes;
                while (rec != NULL && strcmp(rec->nombre, mejor->nombre) != 0) {
                    rec = rec->siguiente;
                }
                if (rec != NULL && rec->cantidad < rec->maximo) {
                    mejor->cantidad--;
                    rec->cantidad++;
                    printf("  [Autonoma] %s envio 1 unidad de %s a %s.\n",
                           c->nombre, mejor->nombre, dest->nombre);
                }
            }
        }
        c = c->siguiente;
    }
}

int encolar_solicitud_biblioteca(struct Sociedad *sociedad,
                                 struct Comuna *origen,
                                 const char *nombrePersona,
                                 const char *termino) {
    struct SolicitudBiblioteca *nueva = malloc(sizeof(struct SolicitudBiblioteca));
    if (!nueva) return 0;

    nueva->nombrePersona = copiar_texto(nombrePersona);
    nueva->terminoBuscado = copiar_texto(termino);
    if (!nueva->nombrePersona || !nueva->terminoBuscado) {
        free(nueva->nombrePersona);
        free(nueva->terminoBuscado);
        free(nueva);
        return 0;
    }
    nueva->comunaOrigen = origen;
    nueva->siguiente = NULL;

    if (sociedad->colaSolicitudes == NULL) {
        sociedad->colaSolicitudes = nueva;
    } else {
        struct SolicitudBiblioteca *act = sociedad->colaSolicitudes;
        while (act->siguiente != NULL) act = act->siguiente;
        act->siguiente = nueva;
    }
    return 1;
}

void procesar_red_interbibliotecaria(struct Sociedad *sociedad) {
    if (sociedad->colaSolicitudes == NULL) return;

    printf("\n=== PROCESANDO RED INTERBIBLIOTECARIA (COLA DE PRÉSTAMOS) ===\n");
    while (sociedad->colaSolicitudes != NULL) {
        struct SolicitudBiblioteca *sol = sociedad->colaSolicitudes;
        sociedad->colaSolicitudes = sol->siguiente;

        bool resuelta = false;
        struct Comuna *vecina = sociedad->comunas;
        while (vecina != NULL) {
            if (vecina != sol->comunaOrigen && vecina->biblioteca != NULL) {
                // revisa si la vecina ya tiene el termino cacheado en su arbol Splay
                NodoSplay *hit = splay_buscar(vecina->biblioteca->cache_textos,
                                              sol->terminoBuscado, true);
                if (hit != NULL && hit->resultados.cantidad > 0) {
                    printf("  [Red] %s comparte cache de '%s' con %s (para %s).\n",
                           vecina->nombre, sol->terminoBuscado,
                           sol->comunaOrigen->nombre, sol->nombrePersona);
                    // replica el resultado en el Splay Tree de la comuna origen
                    ListaArchivos copia;
                    lista_archivos_inicializar(&copia);
                    NodoArchivo *arch = hit->resultados.cabeza;
                    while (arch != NULL) {
                        lista_archivos_agregar(&copia, arch->nombre_archivo);
                        arch = arch->siguiente;
                    }
                    splay_insertar(sol->comunaOrigen->biblioteca->cache_textos,
                                   sol->terminoBuscado, &copia, true);
                    resuelta = true;
                    break;
                }
            }
            vecina = vecina->siguiente;
        }

        if (!resuelta) {
            printf("  [Red] Ninguna comuna vecina tenia '%s' cacheado aun.\n",
                   sol->terminoBuscado);
        }

        free(sol->nombrePersona);
        free(sol->terminoBuscado);
        free(sol);
    }
}