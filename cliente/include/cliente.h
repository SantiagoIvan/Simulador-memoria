#ifndef CLIENTE_H
#define CLIENTE_H

#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdbool.h>
#include "shared_utils.h"
#include "servidor_utils.h"
#include <pthread.h>
#include "serializacion.h"
//#include "mensajes_cliente.h"
#include "entidades.h"

pthread_t hilo_server;
int socket_server;
t_config *config;
t_log *logger;

char *ip_target;
char *puerto_target;
int id;

void leer_comando(char *linea);

void obtener_restaurante();

void inicializar_modulo();

void catch_signal(int signal);

int iniciar_conexion(char *modulo);

int terminar_programa();

void guardar_plato(char **comando_ingresado);

void plato_listo(char **comando_ingresado);

void finalizar_pedido(char **comando_ingresado);

void obtener_pedido(char **comando_ingresado);

void guardar_pedido(char **comando_ingresado);

void confirmar_pedido(char **comando_ingresado);

void prueba_final_comanda();

void free_palabras(char **, int);

#endif
