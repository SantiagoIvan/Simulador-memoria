#ifndef ENTIDADES_H_
#define ENTIDADES_H_

#include <stdint.h>

typedef enum {
    LIBRE,
    ASIGNADO,
    MOVIENDOSE,
    ESPERANDO_PEDIDO_TERMINADO
} Estado_Repartidor;

typedef enum {
    EN_PREPARACION,
    PREPARADO
} Estado_Plato;

typedef enum {
    PENDIENTE,
    CONFIRMADO,
    TERMINADO
} Estado_Pedido;

typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCK,
    EXIT
} Estado_Pcb;

typedef struct Posicion {
    int x;
    int y;
} Posicion;

typedef struct Repartidor {
    int id_repartidor;
    Posicion* posicion;
    Estado_Repartidor estado;
    int frecuencia_descanso;
    int tiempo_descanso;
    //float ganancia; esperar siguiente errata
} Repartidor;

typedef struct Plato { // equivalente a item_pedido. Equivalente a Frame en Comanda
    int size_nombre;
    char *nombre;
    int cantidad_pedida_por_plato;
    int cantidad_preparada_por_plato;
    //int precio; esperar siguiente errata
} Plato;


typedef struct Receta {
    int size_nombre;
    char *nombre;//equivalente a nombre de plato
    int size_pasos;
    char *pasos;//lista de palabras?, cada una es el paso. Funciones especiales
    char *tiempo_pasos;
    int size_tiempo_pasos;
    //TODO int precio;
} Receta;

typedef struct Restaurante {
    int size_nombre;
    char *nombre;
    int cantidad_cocineros;
    Posicion posicion;
    char *afinidades;
    int size_afinidades;
    char *platos;
    int size_platos;
    char *precio_platos;
    int size_precio_platos;
    //int size_platos_conocidos;
    //char **platos_conocidos;
    //int *precio_por_plato;
    int cantidad_hornos;
} Restaurante;

typedef struct Pedido {
    int id_pedido;
    int size_nombre_restaurante;
    char *nombre_restaurante;
    Estado_Pedido estado;
    int size_platos;
    Plato *platos;
    //int *precios
    int precio_total;//esperar a la siguiente version
} Pedido;

typedef struct Restaurante_Conectado {
    char *ip; //la obtenemos cuando se hace el handshake y la guardamos.
    int size_nombre;
    char *nombre;
    Posicion posicion;
} Restaurante_Conectado;

typedef struct Pedido_Control_Block {
    struct Restaurante_Conectado * restaurante;
    struct Cliente_Conectado * cliente;
    Repartidor *repartidor;
    int id_pedido;//me lo da el restaurante
    int proxima_parada;
    int tiempo_espera_ready;
    int rafaga_anterior;
    double estimacion_sjf;
    //seguro falta algun campo relacionado con Información sobre I/O
    struct Pedido_Control_Block *siguiente;
} Pedido_Control_Block;

typedef struct Plato_Control_Block {
    int cocinero_id;
    Estado_Pcb estado;
    int id_pedido;
    //seguro falta algun campo relacionado con Información sobre I/O
    struct Plato_Control_Block *siguiente;
} Plato_Control_Block;

typedef struct Cliente_Conectado {
    char *ip;
    Posicion posicion;
    int id;
    Restaurante_Conectado *restaurante_seleccionado;
} Cliente_Conectado;

#endif