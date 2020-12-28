#ifndef COMANDA_H
#define COMANDA_H

#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/collections/dictionary.h>
#include<readline/readline.h>
#include<readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "entidades.h"
#include "servidor_utils.h"
#include <pthread.h>
#include <time.h>

#define TAMANIO_PAGINA 32
#define TAMANIO_NOMBRE_PLATO 24

char *ruta_config = "cfg/comanda.config";


t_log *logger;
t_config *config;

int socket_server;
pthread_t hilo_server;

typedef struct Pagina {
    uint32_t cantidad_total;
    uint32_t cantidad_lista;
    char *nombre_plato;
} Pagina;

void *memoria_principal;
void *memoria_virtual;
bool *bitmap_memoria_principal;
bool *bitmap_memoria_virtual;

pthread_mutex_t lock_bitmap_mp;
pthread_mutex_t lock_bitmap_mv;

pthread_mutex_t lock_swap;
pthread_mutex_t lock_paginas_para_swapeo;

int cantidad_de_frames_mp;
int cantidad_de_frames_mv;

//lista para hacer el swap
t_list *paginas_para_swapeo;
int index_puntero_cm = 0;
int cantidad_de_swapeos = 0;
char *algoritmo_swap;

t_dictionary *tablas_de_restaurantes;

typedef struct Info_Pagina_Plato{
    int nro_de_frame_memoria_virtual;
    int nro_de_frame_memoria_principal;
    bool bit_de_presencia;
    bool bit_de_modificacion;
    bool bit_de_uso;
    bool bit_de_lockeo;
    pthread_mutex_t lock_pagina;
} Info_Pagina_Plato;

typedef struct Segmento_Pedido{
    Estado_Pedido estado;
    t_dictionary *tabla_de_platos;
} Segmento_Pedido;

typedef enum{
    CARGA,
    REFERENCIA
} Tipo_Operacion;

void mostrar_memoria_principal();

void mostrar_memoria_virtual();

void inicializar_modulo();

void inicializar_memoria();

void *escuchar_mensajes(void *);

void *escuchar_conexiones(void *);

void guardar_pedido(Guardar_Pedido *pedido, int conexion);

void guardar_plato(Plato_Pedido *plato, int conexion);

void obtener_pedido(Obtener_Pedido *pedido, int conexion);

void confirmar_pedido(Confirmar_Pedido *pedido, int conexion);

void plato_listo(Plato_Pedido *plato, int conexion);

void finalizar_pedido(Finalizar_Pedido *pedido, int conexion);

void free_palabras(char **palabras,int size);

int terminar_programa();

void catch_signal(int signal);

void crear_tabla_de_pedidos(char *resto, int id_pedido);

bool existe_restaurante_en_memoria(char *restaurante);

Segmento_Pedido *existe_pedido_de_restaurante(char *restaurante, int id_pedido);

t_dictionary *obtener_pedidos_de_restaurante(char *restaurante);

void agregar_pedido_a_restaurante(Guardar_Pedido *restaurante);

Estado_Pedido obtener_estado_de_pedido(char *restaurante, int id_pedido);

Segmento_Pedido *obtener_pedido_de_restaurante(char *restaurante, int id_pedido);

int agregar_plato_a_pedido(char *plato, Segmento_Pedido *pedido, int cantidad);

int obtener_frame_libre_mp();

int obtener_frame_libre_mv();

void leer_plato_de_memoria(void *memoria, int frame);

void guardar_plato_en_memoria(void *memoria, int frame, Pagina *plato);

Pagina *crear_plato(char *plato, int cantidad);

Info_Pagina_Plato *crear_pagina();

void sumar_cantidad_total(Info_Pagina_Plato *plato, int cantidad);

void sumar_cantidad_lista(Info_Pagina_Plato *pagina);

void agregar_nueva_plato_a_pedido(Segmento_Pedido *pedido, char *plato, int cantidad, int nro_de_frame);

Pagina *obtener_plato_de_memoria(void *memoria, int nro_de_frame);

void pagina_modificada(Info_Pagina_Plato *pagina);

void pagina_leida(Info_Pagina_Plato *pagina);

void pagina_levantada_en_memoria_principal(Info_Pagina_Plato *pagina, int frame);

void levantar_pagina_en_memoria(char *plato, Info_Pagina_Plato *pagina);

void liberar_pagina(Info_Pagina_Plato *pagina);

void liberar_paginas_de_pedido(Segmento_Pedido *segmento);

void destruir_restaurante(t_dictionary *tabla_de_pedidos);

void destruir_pedido(Segmento_Pedido *pedido);

//----------------SWAP------------------//
bool esta_unlockeada(Info_Pagina_Plato *pagina);

void setear_bit_de_lockeo(char *plato, Info_Pagina_Plato *pag);

void resetear_bit_de_lockeo(char *plato, Info_Pagina_Plato *pag);

void mostrar_pagina(Info_Pagina_Plato *pagina);

bool es_0_0(Info_Pagina_Plato *pagina);

bool es_0_0(Info_Pagina_Plato *pagina);

void agregar_a_lista_de_swapeo(Info_Pagina_Plato *pagina, Tipo_Operacion op);

Info_Pagina_Plato *seleccion_de_victima_lru();

Info_Pagina_Plato *seleccion_de_victima_clock_modificado(Info_Pagina_Plato *pagina);

int swap(Info_Pagina_Plato *pagina);

void lockear_pagina(Info_Pagina_Plato *pag);

void unlockear_pagina(Info_Pagina_Plato *pag);

void lockear_paginas_en_memoria();

void unlockear_paginas_en_memoria();

Info_Pagina_Plato *circular_list_get_by_condition(Info_Pagina_Plato *pagina_requerida, bool (*f)(void*));

int indice_siguiente_lista_circular(int indice_actual);

//--------------------------------------------//

void leer_comando(char *linea);//solo para mostrar la MP, MV y Cola de Paginas de Swapeo.

#endif
