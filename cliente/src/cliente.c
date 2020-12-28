#include "cliente.h"
#include <sysexits.h>
#include <signal.h>

int main(void) {
    char *linea;
    signal(SIGINT, catch_signal);
    inicializar_modulo();
    log_info(logger, "CLIENTE: Iniciado");

    while (1) {    
        linea = readline(">");
        if (!strcmp(linea,"")) continue;
        add_history(linea);
        leer_comando(linea);
        free(linea);
    }
    terminar_programa();
}

void inicializar_modulo(){
    config = config_create("cfg/cliente.config");
    logger = log_create(config_get_string_value(config, "ARCHIVO_LOG"), "CLIENTE", true, LOG_LEVEL_INFO);
        
    ip_target = config_get_string_value(config, "IP_COMANDA");
    puerto_target = config_get_string_value(config, "PUERTO_COMANDA");

    return;
}


void leer_comando(char *linea) {

    char **palabras = string_split(linea, " ");
    Codigo_Operacion operacion = get_codigo_operacion_by_string(palabras[0]);

    switch (operacion) {
        case GUARDAR_PEDIDO:
            guardar_pedido(palabras);
            break;
        case GUARDAR_PLATO:
            guardar_plato(palabras);
            break;
        case OBTENER_PEDIDO:
            obtener_pedido(palabras);
            break;
        case CONFIRMAR_PEDIDO:
            confirmar_pedido(palabras);
            break;
        case PLATO_LISTO:
            plato_listo(palabras);
            break;
        case FINALIZAR_PEDIDO:
            finalizar_pedido(palabras);
            break;
        case SALIR:
            free(linea);
            free_palabras(palabras, 1);
            terminar_programa();
        case PRUEBA_FINAL_COMANDA:
            prueba_final_comanda();
            break;
        default:
            log_error(logger, "CLIENTE: comando invalido");
    }
    return;
}

void guardar_pedido(char **comando_ingresado){
    char *resto = comando_ingresado[1];
    int id_pedido = atoi(comando_ingresado[2]);
    log_info(logger, "CLIENTE: guardar_pedido %s %i", resto, id_pedido);

    int conexion = crear_conexion(ip_target, puerto_target);
    Mensaje *mensaje = crear_mensaje(GUARDAR_PEDIDO);
    serializar_guardar_pedido(mensaje, id_pedido, resto);
    enviar_mensaje(mensaje, conexion);

    Descripcion *respuesta = recibir_descripcion(conexion);
    if(respuesta->result){
        log_info(logger, "CLIENTE: GUARDAR_PEDIDO: Ok");
    }else
    {
        log_error(logger, "CLIENTE: GUARDAR_PEDIDO: %s", respuesta->descripcion);
    }
    
    free_palabras(comando_ingresado, 3);
    free(respuesta->descripcion);
    free(respuesta);
    liberar_conexion(conexion);
}

void guardar_plato(char **comando_ingresado){
    char *resto = comando_ingresado[1];
    int id_pedido = atoi(comando_ingresado[2]);
    char *plato = comando_ingresado[3];
    int cant = atoi(comando_ingresado[4]);

    log_info(logger, "CLIENTE: comando guardar_plato %s %i %s %i", resto, id_pedido, plato, cant);
    int conexion = crear_conexion(ip_target, puerto_target);
    Mensaje *mensaje = crear_mensaje(GUARDAR_PLATO);
    serializar_guardar_plato(mensaje, resto, id_pedido, plato, cant);
    enviar_mensaje(mensaje, conexion);

    Descripcion *respuesta = recibir_descripcion(conexion);
    if(respuesta->result){
        log_info(logger, "CLIENTE: GUARDAR_PLATO: Ok");
    }else
    {
        log_error(logger, "CLIENTE: GUARDAR_PLATO: %s", respuesta->descripcion);
    }

    free_palabras(comando_ingresado, 4);
    liberar_conexion(conexion);
}

void obtener_pedido(char **comando_ingresado){
    char *resto = comando_ingresado[1];
    int id_pedido = atoi(comando_ingresado[2]);

    void liberar_plato(Plato *plato){
        free(plato->nombre);
        free(plato);
    }
    int conexion = crear_conexion(ip_target, puerto_target);
    Mensaje *mensaje = crear_mensaje(OBTENER_PEDIDO);
    serializar_obtener_pedido(mensaje, resto, id_pedido);
    enviar_mensaje(mensaje, conexion);

    Descripcion *descripcion = recibir_descripcion(conexion);

    if (descripcion->result){
        log_info(logger, "CLIENTE: OBTENER_PEDIDO: Ok");
        Codigo_Operacion cod = recibir_operacion(conexion);
        Payload *payload = (Payload *) malloc(sizeof(Payload));
        payload->stream = recibir_payload(&(payload->size), conexion);

        t_list *platos = list_create();
        Estado_Pedido estado;
        deserializar_respuesta_obtener_pedido(payload->stream, platos, &estado);
        log_info(logger, "Estado del pedido: %i", estado);
        for(int i=0; i<list_size(platos); i++){
            printf("\n");
            Plato *plato = list_get(platos, i);
            log_info(logger, "Nombre del plato %s: ", plato->nombre);
            log_info(logger, "Cantidad pedida: %i: ", plato->cantidad_pedida_por_plato);
            log_info(logger, "Cantidad preparada %i: ", plato->cantidad_preparada_por_plato);
            printf("\n");
        }
        list_destroy_and_destroy_elements(platos, liberar_plato);
    }else
    {
        log_error(logger, "CLIENTE: OBTENER_PEDIDO: %s", descripcion->descripcion);
    }

    free(descripcion->descripcion);
    free(descripcion);
    liberar_conexion(conexion);
}

void confirmar_pedido(char **comando_ingresado){
    int id_pedido = atoi(comando_ingresado[1]);
    char *nombre_restaurante = comando_ingresado[2];

    log_info(logger, "CLIENTE: Confirmar_Pedido %i %s", id_pedido, nombre_restaurante);

    int conexion = crear_conexion(ip_target, puerto_target);

    Mensaje *mensaje = crear_mensaje(CONFIRMAR_PEDIDO);
    serializar_confirmar_pedido(mensaje, id, id_pedido, nombre_restaurante);
    enviar_mensaje(mensaje, conexion);

    Descripcion *respuesta = recibir_descripcion(conexion);
    if(respuesta->result){
        log_info(logger, "CLIENTE: CONFIRMAR_PEDIDO: Ok");
    }else
    {
        log_error(logger, "CLIENTE: CONFIRMAR_PEDIDO: %s", respuesta->descripcion);
    }
    free_palabras(comando_ingresado, 3);
    free(respuesta->descripcion);
    free(respuesta);
    liberar_conexion(conexion);
}

void plato_listo(char **comando_ingresado){
    char *resto = comando_ingresado[1];
    int id_pedido = atoi(comando_ingresado[2]);
    char *plato = comando_ingresado[3];
    log_info(logger, "CLIENTE: Plato_Listo %s %i %s", resto, id_pedido, plato);

    int conexion = crear_conexion(ip_target, puerto_target);

    Mensaje *mensaje = crear_mensaje(PLATO_LISTO);
    serializar_plato_listo(mensaje, resto, id_pedido, plato);
    enviar_mensaje(mensaje, conexion);

    Descripcion *respuesta = recibir_descripcion(conexion);
    if(respuesta->result){
        log_info(logger, "CLIENTE: PLATO_LISTO: Ok");
    }else
    {
        log_error(logger, "CLIENTE: PLATO_LISTO: %s", respuesta->descripcion);
    }
    free_palabras(comando_ingresado, 4);
    free(respuesta->descripcion);
    free(respuesta);
    liberar_conexion(conexion);
}

void finalizar_pedido(char **comando_ingresado){
    char *resto = comando_ingresado[1];
    int id_pedido = atoi(comando_ingresado[2]);
    log_info(logger, "CLIENTE: Finalizar_ Pedido %s %i",resto, id_pedido);

    int conexion = crear_conexion(ip_target, puerto_target);

    Mensaje *mensaje = crear_mensaje(FINALIZAR_PEDIDO);
    serializar_finalizar_pedido(mensaje, resto, id_pedido);
    enviar_mensaje(mensaje, conexion);

    Descripcion *respuesta = recibir_descripcion(conexion);
    if(respuesta->result){
        log_info(logger, "CLIENTE: FINALIZAR_PEDIDO: Ok");
    }else
    {
        log_error(logger, "CLIENTE: FINALIZAR_PEDIDO: %s", respuesta->descripcion);
    }


    free_palabras(comando_ingresado, 3);
    free(respuesta->descripcion);
    free(respuesta);
    liberar_conexion(conexion);
    return;
}

void free_palabras(char **palabras, int size) {
    for (int i = 0; i < size; i++)
        free(palabras[i]);

    free(palabras);

    return;
}

void catch_signal(int signal) {
    terminar_programa();
}

int terminar_programa() {
    log_info(logger, "CLIENTE: Exit");
    log_destroy(logger);
    config_destroy(config);
    exit(0);
}

void prueba_final_comanda(){
    guardar_pedido(string_split("guardar_pedido boka 1", " "));
    guardar_pedido(string_split("guardar_pedido boka 2", " "));
    guardar_pedido(string_split("guardar_pedido boka 3", " "));
    guardar_pedido(string_split("guardar_pedido boka 4", " "));
    guardar_pedido(string_split("guardar_pedido boka 5", " "));
    guardar_pedido(string_split("guardar_pedido boka 6", " "));
    guardar_pedido(string_split("guardar_pedido boka 7", " "));
    sleep(1);
    guardar_plato(string_split("guardar_plato boka 1 asadoconfritas 1", " "));
    sleep(1);
    guardar_plato(string_split("guardar_plato boka 2 asadoconfritas 2", " "));
    sleep(1);
    guardar_plato(string_split("guardar_plato boka 5 bondiolita 1", " "));
    sleep(1);
    guardar_plato(string_split("guardar_plato boka 3 asadoconfritas 1", " "));
    sleep(1);
    guardar_plato(string_split("guardar_plato boka 6 bondiolita 4", " "));
    sleep(1);
    obtener_pedido(string_split("obtener_pedido boka 1", " "));
    sleep(1);
    guardar_plato(string_split("guardar_plato boka 4 choripan 20", " "));
    sleep(1);
    obtener_pedido(string_split("obtener_pedido boka 2", " "));
    sleep(1);
    guardar_plato(string_split("guardar_plato boka 7 morcipan 1", " "));
    sleep(1);
    guardar_plato(string_split("guardar_plato boka 1 asadoconfritas 1", " "));
    sleep(1);
    guardar_plato(string_split("guardar_plato boka 3 asadoconfritas 1", " "));
    sleep(1);
    obtener_pedido(string_split("obtener_pedido boka 3", " "));
    sleep(1);
    confirmar_pedido(string_split("confirmar_pedido 1 boka", " "));
    sleep(1);
    confirmar_pedido(string_split("confirmar_pedido 2 boka", " "));
    sleep(1);
    confirmar_pedido(string_split("confirmar_pedido 3 boka", " "));
    sleep(1);
    confirmar_pedido(string_split("confirmar_pedido 4 boka", " "));
    sleep(1);
    confirmar_pedido(string_split("confirmar_pedido 5 boka", " "));
    sleep(1);
    confirmar_pedido(string_split("confirmar_pedido 6 boka", " "));
    sleep(1);
    confirmar_pedido(string_split("confirmar_pedido 7 boka", " "));
    sleep(1);
    plato_listo(string_split("plato_listo boka 1 asadoconfritas", " "));
    sleep(1);
    plato_listo(string_split("plato_listo boka 2 asadoconfritas", " "));
    sleep(1);
    plato_listo(string_split("plato_listo boka 3 asadoconfritas", " "));
    sleep(1);
    plato_listo(string_split("plato_listo boka 1 asadoconfritas", " "));
    sleep(1);
    plato_listo(string_split("plato_listo boka 2 asadoconfritas", " "));
    sleep(1);
    plato_listo(string_split("plato_listo boka 5 bondiolita", " "));
    sleep(1);
    plato_listo(string_split("plato_listo boka 6 bondiolita", " "));
    plato_listo(string_split("plato_listo boka 6 bondiolita", " "));
    plato_listo(string_split("plato_listo boka 6 bondiolita", " "));
    plato_listo(string_split("plato_listo boka 6 bondiolita", " "));
    sleep(1);
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    plato_listo(string_split("plato_listo boka 4 choripan", " "));
    sleep(1);
    plato_listo(string_split("plato_listo boka 7 morcipan", " "));
    sleep(10);
    finalizar_pedido(string_split("finalizar_pedido boka 1", " "));
    finalizar_pedido(string_split("finalizar_pedido boka 2", " "));
    finalizar_pedido(string_split("finalizar_pedido boka 3", " "));
    finalizar_pedido(string_split("finalizar_pedido boka 4", " "));
    finalizar_pedido(string_split("finalizar_pedido boka 5", " "));
    finalizar_pedido(string_split("finalizar_pedido boka 6", " "));
    finalizar_pedido(string_split("finalizar_pedido boka 7", " "));
}