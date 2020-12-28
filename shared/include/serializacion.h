#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include"entidades.h"
#include"shared_utils.h"
#include<commons/string.h>
#include<commons/collections/list.h>
#include<commons/collections/dictionary.h>
#include"servidor_utils.h"

typedef struct {
    int id_pedido;
    int size_nombre;
    char *restaurante;
} Finalizar_Pedido, Guardar_Pedido, Obtener_Pedido, Confirmar_Pedido, Seleccionar_Restaurante;

typedef struct Descripcion{
    int size;
    bool result;
    char *descripcion;
} Descripcion;

typedef struct Plato_Pedido{
    int size_nombre_restaurante;
    char *nombre_restaurante;
    int id_pedido;
    int size_nombre_plato;
    char *nombre_plato;
    int cantidad_pedida;
} Plato_Pedido, Plato_Listo;

void serializar_int(void *stream, int n, int *desplazamiento);

void deserializar_int(int *destino, void *stream, int *desplazamiento);

void serializar_string(void *stream, int size, char *string, int *desplazamiento);

void deserializar_string(int *size_destino, char **string_destino, void *stream, int *desplazamiento);

void serializar_guardar_pedido(Mensaje *mensaje, int id_pedido, char *nombre);

void deserializar_guardar_pedido(void *stream, Guardar_Pedido *pedido);

void deserializar_descripcion(void *stream, Descripcion *descripcion);

void serializar_descripcion(Mensaje *mensaje, bool result, char *descripcion);

void serializar_guardar_plato(Mensaje *mensaje, char *restaurante, int id_pedido, char *plato, int cantidad);

void deserializar_guardar_plato(void *stream, Plato_Pedido *plato);

void serializar_obtener_pedido(Mensaje *mensaje, char *nombre_restaurnte, int id_pedido);

void deserializar_obtener_pedido(void *stream, Obtener_Pedido *pedido);

void serializar_respuesta_obtener_pedido(Mensaje *mensaje, t_list *, Estado_Pedido);

void deserializar_respuesta_obtener_pedido(void *stream, t_list *, Estado_Pedido*);

void serializar_confirmar_pedido(Mensaje *mensaje, int id_cliente, int id_pedido, char *restaurante);//restaurante puede ser null

void deserializar_confirmar_pedido(void *stream, Confirmar_Pedido *pedido, bool incluir_restaurante);

void serializar_plato_listo(Mensaje *mensaje, char *restaurante, int id_pedido, char *plato);

void deserializar_plato_listo(void *stream, Plato_Pedido *plato);

void serializar_finalizar_pedido(Mensaje *mensaje, char *restaurante, int id_pedido);

void deserializar_finalizar_pedido(void *stream, Finalizar_Pedido *pedido);

Descripcion *recibir_descripcion(int conexion);

void enviar_descripcion(bool result, char *detalle, Codigo_Operacion cod, int conexion);

#endif