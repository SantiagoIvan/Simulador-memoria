#include "serializacion.h"

void serializar_int(void *stream, int n, int *desplazamiento){
    memcpy(stream + (*desplazamiento), &n, sizeof(int));
    *desplazamiento += sizeof(int);
}

void deserializar_int(int *destino, void *stream, int *desplazamiento){
    memcpy(destino, stream + (*desplazamiento), sizeof(int));
    *desplazamiento += sizeof(int);
}

void serializar_string(void *stream, int size, char *string, int *desplazamiento){
    serializar_int(stream, size, desplazamiento);
    memcpy(stream + (*desplazamiento), string, strlen(string)+1);
	(*desplazamiento) += strlen(string)+1;
}

void deserializar_string(int *size_destino, char **string_destino, void *stream, int *desplazamiento){
    deserializar_int(size_destino, stream, desplazamiento);
    *string_destino = (char *) malloc((*size_destino) + 1);
	memcpy(*string_destino, stream + (*desplazamiento), (*size_destino) + 1);
	(*desplazamiento) += (*size_destino) + 1;
}

void serializar_descripcion(Mensaje *mensaje, bool result, char *descripcion){
    int desplazamiento = 0;
    if(result){
        mensaje->payload->size = sizeof(int);
        mensaje->payload->stream = malloc(mensaje->payload->size);
        serializar_int(mensaje->payload->stream, result, &desplazamiento);
        return;
    }else
    {
        int size_descripcion = strlen(descripcion);
        mensaje->payload->size = 2*sizeof(int) + strlen(descripcion) + 1;
        mensaje->payload->stream = malloc(mensaje->payload->size);
        serializar_int(mensaje->payload->stream, result, &desplazamiento);
        serializar_string(mensaje->payload->stream, size_descripcion, descripcion, &desplazamiento);
        return;
    }
}

void deserializar_descripcion(void *stream, Descripcion *descripcion){
    int desplazamiento = 0;
    deserializar_int(&(descripcion->result), stream, &desplazamiento);
    if(descripcion->result == false){
        deserializar_string(&(descripcion->size), &(descripcion->descripcion), stream, &desplazamiento);
    }
    return;
}

void serializar_guardar_pedido(Mensaje *mensaje, int id_pedido, char *restaurante){
    int desplazamiento = 0;

    mensaje->payload->size = 2*sizeof(int) + strlen(restaurante)+1;
    mensaje->payload->stream = malloc(mensaje->payload->size);
    serializar_string(mensaje->payload->stream, strlen(restaurante), restaurante, &desplazamiento);
    serializar_int(mensaje->payload->stream, id_pedido, &desplazamiento);
}

void deserializar_guardar_pedido(void *stream, Guardar_Pedido *pedido){
    int desplazamiento = 0;
    deserializar_string(&(pedido->size_nombre), &(pedido->restaurante), stream, &desplazamiento);
    deserializar_int(&(pedido->id_pedido), stream, &desplazamiento);
}

void serializar_guardar_plato(Mensaje *mensaje, char *restaurante, int id_pedido, char *plato, int cantidad){
    int desplazamiento = 0;

    mensaje->payload->size = 4*sizeof(int) + strlen(restaurante) + strlen(plato) + 2;
    mensaje->payload->stream = malloc(mensaje->payload->size);
    serializar_string(mensaje->payload->stream, strlen(restaurante), restaurante, &desplazamiento);
    serializar_int(mensaje->payload->stream, id_pedido, &desplazamiento);
    serializar_string(mensaje->payload->stream, strlen(plato), plato, &desplazamiento);
    serializar_int(mensaje->payload->stream, cantidad, &desplazamiento);
}

void deserializar_guardar_plato(void *stream, Plato_Pedido *plato){
    int desplazamiento = 0;
    deserializar_string(&(plato->size_nombre_restaurante), &(plato->nombre_restaurante), stream, &desplazamiento);
    deserializar_int(&(plato->id_pedido), stream, &desplazamiento);
    deserializar_string(&(plato->size_nombre_plato), &(plato->nombre_plato), stream, &desplazamiento);
    deserializar_int(&(plato->cantidad_pedida), stream, &desplazamiento);
}

void serializar_obtener_pedido(Mensaje *mensaje, char *nombre_restaurante, int id_pedido){
    int desplazamiento = 0;

    mensaje->payload->size = 2*sizeof(int) + strlen(nombre_restaurante) + 1;
    mensaje->payload->stream = malloc(mensaje->payload->size);
    serializar_string(mensaje->payload->stream, strlen(nombre_restaurante), nombre_restaurante, &desplazamiento);
    serializar_int(mensaje->payload->stream, id_pedido, &desplazamiento);
}

void deserializar_obtener_pedido(void *stream, Obtener_Pedido *plato){
    int desplazamiento = 0;
    deserializar_string(&(plato->size_nombre), &(plato->restaurante), stream, &desplazamiento);
    deserializar_int(&(plato->id_pedido), stream, &desplazamiento);
}

void serializar_respuesta_obtener_pedido(Mensaje *mensaje, t_list *platos, Estado_Pedido estado){
    int desplazamiento = 0;
    int cantidad_de_items = list_size(platos);

    mensaje->payload->size = 2*sizeof(int);
    mensaje->payload->stream = malloc(mensaje->payload->size);
    serializar_int(mensaje->payload->stream, estado, &desplazamiento);
    serializar_int(mensaje->payload->stream, cantidad_de_items, &desplazamiento);

    for(int i = 0; i<cantidad_de_items; i++){
        Plato *plato = (Plato *) list_get(platos, i);
        mensaje->payload->size += 3*sizeof(int) + strlen(plato->nombre) + 1;
        mensaje->payload->stream = realloc(mensaje->payload->stream, mensaje->payload->size);
        serializar_string(mensaje->payload->stream, strlen(plato->nombre), plato->nombre, &desplazamiento);
        serializar_int(mensaje->payload->stream, plato->cantidad_pedida_por_plato, &desplazamiento);
        serializar_int(mensaje->payload->stream, plato->cantidad_preparada_por_plato, &desplazamiento);
    }
}

void deserializar_respuesta_obtener_pedido(void *stream, t_list *platos, Estado_Pedido *estado){
    int desplazamiento = 0;
    int cantidad_de_platos;

    deserializar_int(estado, stream, &desplazamiento);
    deserializar_int(&cantidad_de_platos, stream, &desplazamiento);
    for(int i = 0; i<cantidad_de_platos; i++){
        Plato *plato = (Plato *) malloc(sizeof(Plato));

        deserializar_string(&(plato->size_nombre), &(plato->nombre), stream, &desplazamiento);
        deserializar_int(&(plato->cantidad_pedida_por_plato), stream, &desplazamiento);
        deserializar_int(&(plato->cantidad_preparada_por_plato), stream, &desplazamiento);
        list_add(platos, plato);
    }
}

void serializar_confirmar_pedido(Mensaje *mensaje, int id_cliente, int id_pedido, char *restaurante){
    int desplazamiento = 0;

    mensaje->payload->size = sizeof(int);
    mensaje->payload->stream = malloc(mensaje->payload->size);
    serializar_int(mensaje->payload->stream, id_pedido, &desplazamiento);
    if(restaurante) {
        mensaje->payload->size += sizeof(int)*2 + strlen(restaurante) + 1;
        mensaje->payload->stream = realloc(mensaje->payload->stream, mensaje->payload->size);
        serializar_string(mensaje->payload->stream, strlen(restaurante), restaurante, &desplazamiento);
    }
    
    serializar_int(mensaje->payload->stream, id_cliente, &desplazamiento);    
}

void deserializar_confirmar_pedido(void *stream, Confirmar_Pedido *pedido, bool incluir_restaurante){
    int desplazamiento = 0;

    deserializar_int(&(pedido->id_pedido), stream, &desplazamiento);
    if(incluir_restaurante) deserializar_string(&(pedido->size_nombre), &(pedido->restaurante), stream, &desplazamiento);
}

void serializar_plato_listo(Mensaje *mensaje, char *restaurante, int id_pedido, char *plato){
    int desplazamiento = 0;

    mensaje->payload->size = 3*sizeof(int) + strlen(restaurante) + strlen(plato) + 2;
    mensaje->payload->stream = malloc(mensaje->payload->size);
    serializar_string(mensaje->payload->stream, strlen(restaurante), restaurante, &desplazamiento);
    serializar_string(mensaje->payload->stream, strlen(plato), plato, &desplazamiento);
    serializar_int(mensaje->payload->stream, id_pedido, &desplazamiento);
}

void deserializar_plato_listo(void *stream, Plato_Listo *plato){
    int desplazamiento = 0;

    deserializar_string(&(plato->size_nombre_restaurante), &(plato->nombre_restaurante), stream, &desplazamiento);
    deserializar_string(&(plato->size_nombre_plato), &(plato->nombre_plato), stream, &desplazamiento);
    deserializar_int(&(plato->id_pedido), stream, &desplazamiento);
}

void serializar_finalizar_pedido(Mensaje *mensaje, char *restaurante, int id_pedido){
    int desplazamiento = 0;

    mensaje->payload->size = 2*sizeof(int) + strlen(restaurante) + 1;
    mensaje->payload->stream = malloc(mensaje->payload->size);
    serializar_string(mensaje->payload->stream, strlen(restaurante), restaurante, &desplazamiento);
    serializar_int(mensaje->payload->stream, id_pedido, &desplazamiento);
}

void deserializar_finalizar_pedido(void *stream, Finalizar_Pedido *pedido){
    int desplazamiento = 0;

    deserializar_string(&(pedido->size_nombre), &(pedido->restaurante), stream, &desplazamiento);
    deserializar_int(&(pedido->id_pedido), stream, &desplazamiento);
}

Descripcion *recibir_descripcion(int conexion){
    Codigo_Operacion cod = recibir_operacion(conexion);
    Payload *payload = (Payload *) malloc(sizeof(Payload));
    Descripcion *descripcion = (Descripcion *) calloc(1, sizeof(Descripcion));
    
    payload->stream = recibir_payload(&(payload->size), conexion);
    deserializar_descripcion(payload->stream, descripcion);
    free(payload->stream);
    free(payload);
    return descripcion;
}

void enviar_descripcion(bool result, char *detalle, Codigo_Operacion cod,int conexion){
    Mensaje *mensaje = crear_mensaje(cod);
    serializar_descripcion(mensaje, result, detalle);
    enviar_mensaje(mensaje, conexion);
}