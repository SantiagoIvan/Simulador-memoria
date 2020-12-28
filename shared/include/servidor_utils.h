#ifndef SERVIDOR_H_
#define SERVIDOR_H_

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
#include"serializacion.h"
#include <pthread.h>


void* recibir_payload(int*, int);

int iniciar_servidor(char*,char*);

int esperar_cliente(int);

t_list* recibir_mensaje(int);

int recibir_operacion(int);

void enviar_identificacion(int conexion, Codigo_Operacion id);

Codigo_Operacion *recibir_identificacion(int conexion, Codigo_Operacion *buffer);

int handshake(char *ip, char *puerto);

void enviar_identificacion_de_cliente(int conexion, int id, int x, int y);

Cliente_Conectado *recibir_identificacion_de_cliente(int conexion, void *stream);

void enviar_identificacion_de_restaurante(int conexion, int size_nombre, char *nombre, int x, int y);

Restaurante_Conectado *recibir_identificacion_de_restaurante(int conexion, void *stream);

void atender_conexion(int conexion_entrante, void *(*f)(void*));

#endif