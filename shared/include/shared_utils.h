#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/collections/list.h>
#include "entidades.h"
#include <netinet/in.h>
#include <arpa/inet.h>

typedef enum {
    CONSULTAR_RESTAURANTES,
    SELECCIONAR_RESTAURANTE,
    OBTENER_RESTAURANTE,
    CONSULTAR_PLATOS,
    CREAR_PEDIDO,
    GUARDAR_PEDIDO,
    ANIADIR_PLATO,
    GUARDAR_PLATO,
    CONFIRMAR_PEDIDO,
    PLATO_LISTO,
    CONSULTAR_PEDIDO,
    OBTENER_PEDIDO,
    FINALIZAR_PEDIDO,
    TERMINAR_PEDIDO,
    OBTENER_RECETA,
    CREAR_RESTAURANTE,
    CREAR_RECETA,
    PRUEBA,
    SALIR,
    CONNECT,
    HANDSHAKE,
    CLIENTE,
    APP,
    RESTAURANTE,
    COMANDA,
    SINDICATO,
    PRUEBA_FINAL_COMANDA
} Codigo_Operacion;

typedef struct {
    int size;
    void *stream;
} Payload;

typedef struct {
    Codigo_Operacion codigo_operacion;
    Payload *payload;
} Mensaje;

int crear_conexion(char *ip, char *puerto);

void liberar_conexion(int socket_cliente);

Mensaje *crear_mensaje(Codigo_Operacion codigo_operacion);

void crear_payload(Mensaje *mensaje);

void enviar_mensaje(Mensaje *mensaje, int socket_cliente);

void *serializar_mensaje(Mensaje *mensaje, int bytes);

Codigo_Operacion get_codigo_operacion_by_string(char *string_codigo_operacion);

void nuevo_cliente_conectado(t_list *lista, int conexion, Cliente_Conectado *cliente);

void liberar_clientes(t_list *lista);

void nuevo_restaurante_conectado(t_list *lista, int conexion, Restaurante_Conectado *rest);

void liberar_restaurantes(t_list *lista);

char *obtener_ip_conexion(int conexion);

t_list* get_list_from_array(char** values);

#endif