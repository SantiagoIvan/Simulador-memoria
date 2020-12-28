#include "comanda.h"

int main(void) {
    char *linea;
    signal(SIGINT, catch_signal);

    inicializar_modulo();
    log_info(logger, "COMANDA: Iniciado");
    while (1) {    
        linea = readline(">");
        if (!strcmp(linea,"")) continue;
        add_history(linea);
        leer_comando(linea);
        free(linea);
    }
    pthread_join(hilo_server,NULL);
    terminar_programa();
}

void inicializar_modulo(){

    config = config_create(ruta_config);
    logger = log_create(config_get_string_value(config, "ARCHIVO_LOG"), "COMANDA", true, LOG_LEVEL_INFO);
    
    //espero la conexion de la App, o del Cliente en caso de testear el modulo
    char *puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    char *ip_comanda = config_get_string_value(config,"IP_COMANDA");
    socket_server = iniciar_servidor(ip_comanda,puerto_escucha);
    pthread_create(&hilo_server,NULL,escuchar_conexiones,&socket_server);

    //inicializo la memoria
    inicializar_memoria();
    
    algoritmo_swap = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
    log_info(logger, "COMANDA: El algoritmo de SWAP es %s", algoritmo_swap);
    paginas_para_swapeo = list_create();
    
    pthread_mutex_init(&lock_paginas_para_swapeo, NULL);
    pthread_mutex_init(&lock_swap, NULL);

    return;
}

void inicializar_memoria(){
    int tamanio_memoria = config_get_int_value(config, "TAMANIO_MEMORIA");
    cantidad_de_frames_mp = tamanio_memoria/TAMANIO_PAGINA;
    memoria_principal = malloc(tamanio_memoria);
    bitmap_memoria_principal = (bool *) malloc(sizeof(bool)*cantidad_de_frames_mp);
    for(int i=0;i<cantidad_de_frames_mp;i++){
        bitmap_memoria_principal[i] = 0;
    }
    pthread_mutex_init(&lock_bitmap_mp, NULL);

    tamanio_memoria = config_get_int_value(config, "TAMANIO_SWAP");
    cantidad_de_frames_mv = tamanio_memoria/TAMANIO_PAGINA;
    memoria_virtual = malloc(tamanio_memoria);
    bitmap_memoria_virtual = (bool *) malloc(sizeof(bool)*cantidad_de_frames_mv);
    for(int i=0;i<cantidad_de_frames_mv;i++){
        bitmap_memoria_virtual[i] = 0;
    }
    pthread_mutex_init(&lock_bitmap_mv, NULL);

    tablas_de_restaurantes = dictionary_create();
}

//SOLO PARA IR TESTEANDO COMO SE VA MODIFICANDO LA MEMORIA, TA CHETO
void leer_comando(char *linea){
    if(string_equals_ignore_case(linea, "mp")){
        mostrar_memoria_principal();
        int cant=0;
        for(int i = 0; i < cantidad_de_frames_mp; i++){
            if(bitmap_memoria_principal[i]==0) cant++;
        }
        printf("Cantidad de frames libres: %i\n", cant);
    }
    if(string_equals_ignore_case(linea, "mv")){
        mostrar_memoria_virtual();
        int cant=0;
        for(int i = 0; i < cantidad_de_frames_mv; i++){
            if(bitmap_memoria_virtual[i]==0) cant++;
        }
        printf("Cantidad de frames libres: %i\n", cant);
    }
    if(string_equals_ignore_case(linea, "ps")){
        printf("Mostrando cola de Paginas para swapear\n");
        list_iterate(paginas_para_swapeo, mostrar_pagina);
        printf("Fin\n\n");
    }
    if(string_equals_ignore_case(linea, "puntero_cm")){
        printf("%i\n", index_puntero_cm);
    }
}

void *escuchar_conexiones(void *socket_server){
    int sv = *((int *) socket_server);
    int conexion_entrante=-1;

    log_info(logger, "COMANDA: Escuchando conexiones...");
    
    while (1)
    {
        conexion_entrante = esperar_cliente(sv);

        if( conexion_entrante < 0 ){
            log_error(logger, "COMANDA: Error al recibir conexión nueva. Saliendo del programa...");            
            terminar_programa();
        }else{
            log_info(logger, "COMANDA: Conexion de la App recibida correctamente!");
            atender_conexion(conexion_entrante, escuchar_mensajes);
        }
    }
}

void *escuchar_mensajes(void *conn){
    int conexion = *((int *) conn);
    free(conn);
    Codigo_Operacion cod = recibir_operacion(conexion);
    Payload *payload = (Payload *) malloc(sizeof(Payload));
    switch (cod)
    {
        case HANDSHAKE:
            log_info(logger, "COMANDA: Realizando Handshake...");
            Mensaje *msg = crear_mensaje(HANDSHAKE);
            enviar_mensaje(msg, conexion);
            log_info(logger, "COMANDA: Handshake Enviado");
            break;
        
        case GUARDAR_PEDIDO:
            payload->stream = recibir_payload(&(payload->size), conexion);
            Guardar_Pedido *pedido = (Guardar_Pedido *) malloc(sizeof(Guardar_Pedido));
            deserializar_guardar_pedido(payload->stream, pedido);
            free(payload->stream);
            guardar_pedido(pedido, conexion);
            break;

        case GUARDAR_PLATO:
            payload->stream = recibir_payload(&(payload->size), conexion);
            Plato_Pedido *plato = (Plato_Pedido *) malloc(sizeof(Plato_Pedido));
            deserializar_guardar_plato(payload->stream, plato);
            free(payload->stream);
            guardar_plato(plato, conexion);
            break;

        case OBTENER_PEDIDO:
            payload->stream = recibir_payload(&(payload->size), conexion);
            Obtener_Pedido *solicitud_pedido = (Obtener_Pedido *) malloc(sizeof(Obtener_Pedido));
            deserializar_obtener_pedido(payload->stream, solicitud_pedido);
            free(payload->stream);
            obtener_pedido(solicitud_pedido, conexion);
            break;

        case CONFIRMAR_PEDIDO:
            payload->stream = recibir_payload(&(payload->size), conexion);
            Confirmar_Pedido *aux2 = (Confirmar_Pedido *) malloc(sizeof(Confirmar_Pedido));
            deserializar_confirmar_pedido(payload->stream, aux2, 1);
            free(payload->stream);
            confirmar_pedido(aux2, conexion);
            break;

        case PLATO_LISTO:
            payload->stream = recibir_payload(&(payload->size), conexion);
            Plato_Pedido *aux3 = (Plato_Pedido *) malloc(sizeof(Plato_Pedido));
            deserializar_plato_listo(payload->stream, aux3);
            free(payload->stream);
            plato_listo(aux3, conexion);
            break;

        case FINALIZAR_PEDIDO:
            payload->stream = recibir_payload(&(payload->size), conexion);
            Finalizar_Pedido *aux = (Finalizar_Pedido *) malloc(sizeof(Finalizar_Pedido));
            deserializar_finalizar_pedido(payload->stream, aux);
            free(payload->stream);
            finalizar_pedido(aux, conexion);
            break;
        default:
            break;
    }
    free(payload);
}

//---------MENSAJERIA-----------------//

void guardar_pedido(Guardar_Pedido *pedido, int conexion) {    

    if( existe_restaurante_en_memoria(pedido->restaurante) ){
        
        if(!existe_pedido_de_restaurante(pedido->restaurante, pedido->id_pedido)){
            agregar_pedido_a_restaurante(pedido);
            log_info(logger, "COMANDA: Nuevo pedido %i agregado al Restaurante %s !", pedido->id_pedido, pedido->restaurante);
            enviar_descripcion(true, NULL, GUARDAR_PEDIDO, conexion);
            liberar_conexion(conexion);
        }else
        {
            log_error(logger, "COMANDA: Ya existe un pedido con ese ID");
            enviar_descripcion(false, "Ya existe un pedido con ese ID\0", GUARDAR_PEDIDO, conexion);
            liberar_conexion(conexion);
        }

    }else
    {
        crear_tabla_de_pedidos(pedido->restaurante, pedido->id_pedido);
        log_info(logger, "COMANDA: Restaurante %s agregado con éxito", pedido->restaurante);
        enviar_descripcion(true, NULL, GUARDAR_PEDIDO, conexion);
        liberar_conexion(conexion);
    }
    
    free(pedido->restaurante);
    free(pedido);
    return;
}

void guardar_plato(Plato_Pedido *plato, int conexion) {
    Segmento_Pedido *segmento;
    void operacion_finalizada(bool result, char *mensaje){
        enviar_descripcion(result, mensaje, GUARDAR_PLATO, conexion);
        liberar_conexion(conexion);
        free(plato->nombre_plato);
        free(plato->nombre_restaurante);
        free(plato);
        return;
    }
    if(!existe_restaurante_en_memoria(plato->nombre_restaurante)){
        char *mensaje = "El restaurante no existe en memoria\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    }
    if( !( segmento = existe_pedido_de_restaurante(plato->nombre_restaurante, plato->id_pedido ))){
        char *mensaje = "El pedido no existe en memoria\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    }
    if( segmento->estado != PENDIENTE){
        char *mensaje = "El pedido ya se confirmo\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    }
    int res = agregar_plato_a_pedido(plato->nombre_plato, segmento, plato->cantidad_pedida);
    if( res < 0 ){
        operacion_finalizada(false, "No se pudo agregar el plato\0");
        return;
    }
    Info_Pagina_Plato *pag = dictionary_get(segmento->tabla_de_platos, plato->nombre_plato);
    Pagina *frame = obtener_plato_de_memoria(memoria_principal, pag->nro_de_frame_memoria_principal);
    log_info(logger, "COMANDA: Almacenado en memoria principal Restaurante: %s | Id_Pedido: %i | Plato: %s | Cantidad_total: %i | Cantidad_lista %i", plato->nombre_restaurante, plato->id_pedido, frame->nombre_plato, frame->cantidad_total, frame->cantidad_lista);
    free(frame->nombre_plato);
    free(frame);
    operacion_finalizada(true, NULL);
    return;
}

void obtener_pedido(Obtener_Pedido *pedido, int conexion) {
    t_list *lista_de_platos = list_create();//lo que voy a serializar
    Segmento_Pedido *segmento;
    void liberar_plato(Plato *plato){
        free(plato->nombre);
        free(plato);
    }
    void liberar_recursos(){
        list_destroy_and_destroy_elements(lista_de_platos, liberar_plato);
        free(pedido->restaurante);
        free(pedido);
        return;
    }
    void enlistar_platos(char *plato, Info_Pagina_Plato *pagina){
        Pagina *frame = obtener_plato_de_memoria(memoria_principal, pagina->nro_de_frame_memoria_principal);
        pagina_leida(pagina);
        Plato *aux = (Plato *) malloc(sizeof(Plato));
        aux->cantidad_pedida_por_plato = frame->cantidad_total;
        aux->cantidad_preparada_por_plato = frame->cantidad_lista;
        aux->nombre = (char *) malloc(TAMANIO_NOMBRE_PLATO);
        aux->nombre = strcpy(aux->nombre, plato);
        list_add(lista_de_platos, aux);
        free(frame->nombre_plato);
        free(frame);
    }
    if(!existe_restaurante_en_memoria(pedido->restaurante)){
        liberar_recursos();
        log_error(logger, "COMANDA: Fail: El restaurante no está cargado en memoria");
        enviar_descripcion(false, "El restaurante no está cargado en memoria\0", OBTENER_PEDIDO, conexion);
        liberar_conexion(conexion);
        return;
    }
    if( !( segmento = existe_pedido_de_restaurante(pedido->restaurante, pedido->id_pedido ))){
        liberar_recursos();
        log_error(logger, "COMANDA: Fail: El pedido no está cargado en memoria");
        enviar_descripcion(false, "El pedido no está cargado en memoria\0", OBTENER_PEDIDO, conexion);
        liberar_conexion(conexion);
        return;
    }

    //explicacion en el doc de decisiones de diseño
    pthread_mutex_lock(&lock_swap);
    lockear_paginas_en_memoria();
    dictionary_iterator(segmento->tabla_de_platos, setear_bit_de_lockeo);
    dictionary_iterator(segmento->tabla_de_platos, levantar_pagina_en_memoria);
    dictionary_iterator(segmento->tabla_de_platos, enlistar_platos);
    dictionary_iterator(segmento->tabla_de_platos, resetear_bit_de_lockeo);
    pthread_mutex_unlock(&lock_swap);
    unlockear_paginas_en_memoria();

    enviar_descripcion(true, NULL, OBTENER_PEDIDO, conexion);
    Mensaje *respuesta = crear_mensaje(OBTENER_PEDIDO);
    serializar_respuesta_obtener_pedido(respuesta, lista_de_platos, segmento->estado);
    enviar_mensaje(respuesta, conexion);
    liberar_recursos();
    log_info(logger, "COMANDA: Enviado OBTENER PEDIDO");
    liberar_conexion(conexion);
    return;
}

void confirmar_pedido(Confirmar_Pedido *pedido, int conexion) {
    Segmento_Pedido *segmento;

    void operacion_finalizada(bool result, char *mensaje){
        enviar_descripcion(result, mensaje, CONFIRMAR_PEDIDO, conexion);
        liberar_conexion(conexion);
        free(pedido->restaurante);
        free(pedido);
        return;
    }

    if(!existe_restaurante_en_memoria(pedido->restaurante)){
        char *mensaje = "El restaurante no existe en memoria\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    }
    if(!( segmento = existe_pedido_de_restaurante(pedido->restaurante, pedido->id_pedido))){
        char *mensaje = "El pedido no existe dentro del restaurante\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    }
    if( segmento->estado != PENDIENTE ){
        char *mensaje = "El pedido ya se encontraba confirmado o finalizado\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    }
    
    segmento->estado = CONFIRMADO;
    log_info(logger, "COMANDA: Restaurante %s -- Pedido %i -- Estado CONFIRMADO",pedido->restaurante, pedido->id_pedido);
    operacion_finalizada(true, NULL);
    return;
}

void plato_listo(Plato_Listo *plato, int conexion) {
    int cantidad_platos_terminados = 0;
    Segmento_Pedido *segmento;
    void operacion_finalizada(bool result, char *mensaje){
        enviar_descripcion(result, mensaje, PLATO_LISTO, conexion);
        liberar_conexion(conexion);
        free(plato->nombre_plato);
        free(plato->nombre_restaurante);
        free(plato);
        return;
    }
    void plato_terminado(char *plato, Info_Pagina_Plato *pagina){
        Pagina *frame = obtener_plato_de_memoria(memoria_principal, pagina->nro_de_frame_memoria_principal);
        pagina_leida(pagina);
        if(frame->cantidad_lista == frame->cantidad_total){
            cantidad_platos_terminados += 1;
        }
        free(frame->nombre_plato);
        free(frame);
    }

    if(!existe_restaurante_en_memoria(plato->nombre_restaurante)){
        char *mensaje = "El restaurante no existe en memoria\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    }
    if(!( segmento = existe_pedido_de_restaurante(plato->nombre_restaurante, plato->id_pedido ))){
        char *mensaje = "El pedido no existe dentro del restaurante\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    } 
    if(!dictionary_has_key(segmento->tabla_de_platos, plato->nombre_plato)){
        char *mensaje = "El plato no existe dentro del pedido\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    }
    if( segmento->estado != CONFIRMADO ){
        char *mensaje = "El pedido no se encuentra confirmado\0";
        operacion_finalizada(false, mensaje);
        log_info(logger, "COMANDA: %s", mensaje);
        return;
    }
    log_info("COMANDA: Plato listo Restaurante: %s | Id_Pedido: %i | Plato: %s", plato->nombre_restaurante, plato->id_pedido,plato->nombre_plato);

    Info_Pagina_Plato *pagina = dictionary_get(segmento->tabla_de_platos, plato->nombre_plato);
    //si bien se modifica solo una pagina, levanto todas para realizar operaciones de lectura.
    pthread_mutex_lock(&lock_swap);
    lockear_paginas_en_memoria();
    dictionary_iterator(segmento->tabla_de_platos, setear_bit_de_lockeo);
    dictionary_iterator(segmento->tabla_de_platos, levantar_pagina_en_memoria);
    sumar_cantidad_lista(pagina);
    dictionary_iterator(segmento->tabla_de_platos, plato_terminado);
    dictionary_iterator(segmento->tabla_de_platos, resetear_bit_de_lockeo);
    unlockear_paginas_en_memoria();
    pthread_mutex_unlock(&lock_swap);

    if(cantidad_platos_terminados == dictionary_size(segmento->tabla_de_platos)){
        log_info(logger, "COMANDA: El pedido esta terminado");
        segmento->estado = TERMINADO;
    }
    enviar_descripcion(true, NULL, PLATO_LISTO, conexion);
    free(plato->nombre_restaurante);
    free(plato->nombre_plato);
    free(plato);
    return;
}

void finalizar_pedido(Finalizar_Pedido *pedido, int conexion) {
    Segmento_Pedido *segmento;
    void operacion_finalizada(bool result, char *mensaje){
        enviar_descripcion(result, mensaje, FINALIZAR_PEDIDO, conexion);
        liberar_conexion(conexion);
        free(pedido->restaurante);
        free(pedido);
        return;
    }
    
    if(!existe_restaurante_en_memoria(pedido->restaurante)){
        char *mensaje = "El restaurante no existe en memoria\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    }
    if(!( segmento = existe_pedido_de_restaurante(pedido->restaurante, pedido->id_pedido ))){
        char *mensaje = "El pedido no existe dentro del restaurante\0";
        operacion_finalizada(false, mensaje);
        log_error(logger, "COMANDA: %s", mensaje);
        return;
    }

    pthread_mutex_lock(&lock_swap);
    t_dictionary *tabla_de_pedidos = obtener_pedidos_de_restaurante(pedido->restaurante);
    char *id = string_itoa(pedido->id_pedido);
    segmento = dictionary_remove(tabla_de_pedidos, id);
    log_info(logger, "COMANDA: Eliminando el pedido %i del restaurante %s", pedido->id_pedido, pedido->restaurante);
    dictionary_destroy_and_destroy_elements(segmento->tabla_de_platos, liberar_pagina);
    free(segmento);
    if(dictionary_size(tabla_de_pedidos) == 0){
        dictionary_remove_and_destroy(tablas_de_restaurantes, pedido->restaurante, dictionary_destroy);
    }
    pthread_mutex_unlock(&lock_swap);

    enviar_descripcion(true, NULL, FINALIZAR_PEDIDO, conexion);
    liberar_conexion(conexion);
    free(id);
    free(pedido->restaurante);
    free(pedido);
    return;
}
//-------------------------------------//

//------FUNCIONES AUXILIARES-----------//

void crear_tabla_de_pedidos(char *resto, int id_pedido){
    dictionary_put(tablas_de_restaurantes, resto, dictionary_create());
    t_dictionary *tabla_de_segmentos = dictionary_get(tablas_de_restaurantes, resto);
    Segmento_Pedido *segmento = (Segmento_Pedido *) malloc(sizeof(Segmento_Pedido));
    segmento->estado = PENDIENTE;
    segmento->tabla_de_platos = dictionary_create();
    char *id = string_itoa(id_pedido);
    dictionary_put(tabla_de_segmentos, id, segmento);
    free(id);
}

bool existe_restaurante_en_memoria(char *restaurante){
    return dictionary_has_key(tablas_de_restaurantes, restaurante);
}

Segmento_Pedido *existe_pedido_de_restaurante(char *restaurante, int id_pedido){
    t_dictionary *tabla_de_pedidos = dictionary_get(tablas_de_restaurantes, restaurante);
    char *id = string_itoa(id_pedido);
    if(dictionary_has_key(tabla_de_pedidos, id)){
        Segmento_Pedido *seg = (Segmento_Pedido *) dictionary_get(tabla_de_pedidos, id);
        free(id);
        return seg;
    }
    free(id);
    return NULL;
}

t_dictionary *obtener_pedidos_de_restaurante(char *restaurante){
    return dictionary_get(tablas_de_restaurantes, restaurante);
}

void agregar_pedido_a_restaurante(Guardar_Pedido *pedido){
    t_dictionary *tabla_de_segmentos = obtener_pedidos_de_restaurante(pedido->restaurante);
    Segmento_Pedido *segmento = (Segmento_Pedido *) malloc(sizeof(Segmento_Pedido));
    segmento->estado = PENDIENTE;
    segmento->tabla_de_platos = dictionary_create();
    char *id = string_itoa(pedido->id_pedido);
    dictionary_put(tabla_de_segmentos, id, segmento);
    free(id);  
}

Estado_Pedido obtener_estado_de_pedido(char *restaurante, int id_pedido){
    Segmento_Pedido *pedido = obtener_pedido_de_restaurante(restaurante, id_pedido);
    return pedido->estado;
}

Segmento_Pedido *obtener_pedido_de_restaurante(char *restaurante, int id_pedido){
    t_dictionary *segmentos = obtener_pedidos_de_restaurante(restaurante);
    char *id = string_itoa(id_pedido);
    Segmento_Pedido *pedido = dictionary_get(segmentos, id);
    free(id);
    return pedido;
}

int agregar_plato_a_pedido(char *plato, Segmento_Pedido *pedido, int cantidad){

    if(dictionary_has_key(pedido->tabla_de_platos, plato)){//existe el plato?
        Info_Pagina_Plato *pagina = dictionary_get(pedido->tabla_de_platos, plato);
        lockear_pagina(pagina);
        if(pagina->bit_de_presencia){//está en MP?
            sumar_cantidad_total(pagina, cantidad);
            unlockear_pagina(pagina);
        }else
        {   
            pthread_mutex_lock(&lock_swap);
            lockear_paginas_en_memoria();
            swap(pagina);
            pthread_mutex_unlock(&lock_swap);
            sumar_cantidad_total(pagina, cantidad);
            unlockear_paginas_en_memoria();
        }
    }else//no existe el plato
    {
        int nro_de_frame_virtual;
        if( (nro_de_frame_virtual = obtener_frame_libre_mv()) < 0 ){//si no hay espacio direccionable, bai
            log_error(logger, "COMANDA: No hay mas espacio en la memoria virtual");
            return -1;
        }
        //lo agrego a memoria virtual
        agregar_nueva_plato_a_pedido(pedido, plato, cantidad, nro_de_frame_virtual);
        
        Info_Pagina_Plato *pagina = dictionary_get(pedido->tabla_de_platos, plato);
        //ver si hay espacio en memoria para agregarlo, o hacer el swap.
        int nro_de_frame_principal;
        pthread_mutex_lock(&lock_swap);
        
        if( (nro_de_frame_principal = obtener_frame_libre_mp()) < 0 ){
            lockear_paginas_en_memoria();
            swap(pagina);
            pthread_mutex_unlock(&lock_swap);
            unlockear_paginas_en_memoria();
        }else
        {
            pthread_mutex_unlock(&lock_swap);
            lockear_pagina(pagina);
            Pagina *frame = obtener_plato_de_memoria(memoria_virtual, nro_de_frame_virtual);
            guardar_plato_en_memoria(memoria_principal, nro_de_frame_principal, frame);
            pagina_levantada_en_memoria_principal(pagina, nro_de_frame_principal);
            agregar_a_lista_de_swapeo(pagina, CARGA);
            unlockear_pagina(pagina);
            free(frame->nombre_plato);
            free(frame);
        }
        
    }
    return 0;
}

void agregar_nueva_plato_a_pedido(Segmento_Pedido *pedido, char *plato, int cantidad, int nro_de_frame){
    Info_Pagina_Plato *pagina = (Info_Pagina_Plato *) malloc(sizeof(Info_Pagina_Plato));
    pagina->nro_de_frame_memoria_virtual = nro_de_frame;
    pagina->bit_de_modificacion = 0;
    pagina->bit_de_presencia = 0;
    pagina->bit_de_uso = 0;
    pagina->nro_de_frame_memoria_principal = -1;
    pagina->bit_de_lockeo = 0;
    pthread_mutex_init(&(pagina->lock_pagina), NULL);

    Pagina *frame = crear_plato(plato, cantidad);
    guardar_plato_en_memoria(memoria_virtual, nro_de_frame, frame);
    dictionary_put(pedido->tabla_de_platos, plato, pagina);
    free(frame->nombre_plato);
    free(frame);
}

//--------OBTENER INFO DE PLATO EN MEMORIA-------//
Pagina *obtener_plato_de_memoria(void *memoria, int frame){
    int frame_objetivo = TAMANIO_PAGINA * frame;
    int desplazamiento = 0;
    Pagina *res = (Pagina *) malloc(sizeof(Pagina));
    res->nombre_plato = (char *) malloc(TAMANIO_NOMBRE_PLATO);

    memcpy(&(res->cantidad_total), memoria + frame_objetivo + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(res->cantidad_lista), memoria + frame_objetivo + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(res->nombre_plato, memoria + frame_objetivo + desplazamiento, TAMANIO_NOMBRE_PLATO);
    
    return res;
}

void *obtener_dato_de_plato(int frame, int desplazamiento, int size){
    void *dato = malloc(size);
    memcpy(dato, memoria_principal + frame*TAMANIO_PAGINA + desplazamiento, size);
    return dato;
}

void levantar_pagina_en_memoria(char *plato, Info_Pagina_Plato *pagina){
    if(pagina->bit_de_presencia){
        log_info(logger, "COMANDA: plato %s en memoria", plato);
        pagina->bit_de_uso = 1;
    }else
    {
        log_info(logger, "COMANDA: plato %s en disco. Revisando si hay espacio en MP", plato);
        int nro_de_frame_mp;
        if( (nro_de_frame_mp = obtener_frame_libre_mp()) < 0){
            log_info(logger, "COMANDA: No hay espacio en la MP. Swapeando...");
            swap(pagina);
            pthread_mutex_lock(&(pagina->lock_pagina));
        }else
        {
            log_info(logger, "COMANDA: Hay espacio en la MP ! Asignando frame...");
            Pagina *frame = obtener_plato_de_memoria(memoria_virtual, pagina->nro_de_frame_memoria_virtual);
            guardar_plato_en_memoria(memoria_principal, nro_de_frame_mp, frame);
            pagina_levantada_en_memoria_principal(pagina, nro_de_frame_mp);
            agregar_a_lista_de_swapeo(pagina, CARGA);

            free(frame->nombre_plato);
            free(frame);
        }
    }
    
}
//------------------------------------//
//------------ACTUALIZAR INFO EN MEMORIA---------//
void sumar_cantidad_total(Info_Pagina_Plato *pagina, int cantidad){
    uint32_t *dato = (uint32_t *) obtener_dato_de_plato(pagina->nro_de_frame_memoria_principal, 0, sizeof(uint32_t));
    (*dato) += cantidad;
    memcpy(memoria_principal + pagina->nro_de_frame_memoria_principal * TAMANIO_PAGINA, dato, sizeof(uint32_t));
    pagina_modificada(pagina);
    agregar_a_lista_de_swapeo(pagina, REFERENCIA);
    free(dato);
}

void sumar_cantidad_lista(Info_Pagina_Plato *pagina){
    int desplazamiento = sizeof(uint32_t);
    uint32_t *dato = (uint32_t *) obtener_dato_de_plato(pagina->nro_de_frame_memoria_principal, desplazamiento, sizeof(uint32_t));
    (*dato) += 1;
    memcpy(memoria_principal + pagina->nro_de_frame_memoria_principal * TAMANIO_PAGINA + desplazamiento, dato, sizeof(uint32_t));
    pagina_modificada(pagina);
    agregar_a_lista_de_swapeo(pagina, REFERENCIA);
    free(dato);
}

void guardar_plato_en_memoria(void *memoria, int frame, Pagina *plato){
    int desplazamiento = 0;
    int frame_objetivo = TAMANIO_PAGINA * frame;

    memcpy(memoria + frame_objetivo + desplazamiento, &(plato->cantidad_total), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(memoria + frame_objetivo + desplazamiento, &(plato->cantidad_lista), sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(memoria + frame_objetivo + desplazamiento, plato->nombre_plato, TAMANIO_NOMBRE_PLATO);
}
//----------------------------------------//
void pagina_modificada(Info_Pagina_Plato *pagina){
    pagina->bit_de_modificacion = 1;
    pagina->bit_de_uso = 1;
}

void pagina_leida(Info_Pagina_Plato *pagina){
    pagina->bit_de_uso = 1;
}

void pagina_leida_iterator(char *plato, Info_Pagina_Plato *pagina){
    pagina->bit_de_uso = 1;
}

void pagina_levantada_en_memoria_principal(Info_Pagina_Plato *pagina, int frame){
    pagina->bit_de_presencia = 1;
    pagina->nro_de_frame_memoria_principal = frame;
    pagina->bit_de_uso = 1;
    pagina->bit_de_modificacion = 0;
}

int obtener_frame_libre_mp(){
    pthread_mutex_lock(&lock_bitmap_mp);
    for(int i = 0; i<cantidad_de_frames_mp; i++){
        if(bitmap_memoria_principal[i] == 0){
            bitmap_memoria_principal[i] = 1;
            pthread_mutex_unlock(&lock_bitmap_mp);
            return i;
        }
    }
    pthread_mutex_unlock(&lock_bitmap_mp);
    return -1;
}

int obtener_frame_libre_mv(){
    pthread_mutex_lock(&lock_bitmap_mv);
    for(int i = 0; i<cantidad_de_frames_mv; i++){
        if(bitmap_memoria_virtual[i] == 0){
            bitmap_memoria_virtual[i] = 1;
            pthread_mutex_unlock(&lock_bitmap_mv);
            return i;
        }
    }
    pthread_mutex_unlock(&lock_bitmap_mv);
    return -1;
}

Pagina *crear_plato(char *plato, int cantidad){
    Pagina *frame = (Pagina *) malloc(sizeof(Pagina));
    uint32_t valor_inicial = cantidad;
    frame->cantidad_lista = 0;
    frame->cantidad_total = valor_inicial;
    frame->nombre_plato = (char *) malloc(TAMANIO_NOMBRE_PLATO);
    strcpy(frame->nombre_plato, plato);

    return frame;
}

void liberar_pagina(Info_Pagina_Plato *pagina){
    bool pagina_existente_en_lista(Info_Pagina_Plato *pagina_target){
        if(pagina->nro_de_frame_memoria_virtual == pagina_target->nro_de_frame_memoria_virtual){
            return true;
        }
        return false;
    }
    pthread_mutex_lock(&lock_paginas_para_swapeo);
    list_remove_by_condition(paginas_para_swapeo, pagina_existente_en_lista);
    pthread_mutex_unlock(&lock_paginas_para_swapeo);

    //marcar como libre el bitmap
    pthread_mutex_lock(&lock_bitmap_mv);
    bitmap_memoria_virtual[pagina->nro_de_frame_memoria_virtual] = 0;
    pthread_mutex_unlock(&lock_bitmap_mv);
    pthread_mutex_lock(&lock_bitmap_mp);
    if(pagina->nro_de_frame_memoria_principal >= 0){
        
        bitmap_memoria_principal[pagina->nro_de_frame_memoria_principal] = 0;
        log_info(logger, "COMANDA: Liberado el frame %i de Memoria Principal", pagina->nro_de_frame_memoria_principal);

    }
    log_info(logger, "COMANDA: Liberado el frame %i de Memoria Virtual", pagina->nro_de_frame_memoria_virtual);
    pthread_mutex_unlock(&lock_bitmap_mp);
    pthread_mutex_destroy(&(pagina->lock_pagina));
    free(pagina);
}

void lockear_pagina(Info_Pagina_Plato *pag){
    pthread_mutex_lock(&(pag->lock_pagina));
}

void unlockear_pagina(Info_Pagina_Plato *pag){
    pthread_mutex_unlock(&(pag->lock_pagina));
}

//-------------------SWAPEO----------------------------------//

void setear_bit_de_lockeo(char *plato, Info_Pagina_Plato *pag){
    pag->bit_de_lockeo = 1;
}

void resetear_bit_de_lockeo(char *plato, Info_Pagina_Plato *pag){
    pag->bit_de_lockeo = 0;
}

void agregar_a_lista_de_swapeo(Info_Pagina_Plato *pagina, Tipo_Operacion op){
    
    bool pagina_existente_en_lista(Info_Pagina_Plato *pagina_target){
        return pagina_target->nro_de_frame_memoria_virtual == pagina->nro_de_frame_memoria_virtual;
    }
    pthread_mutex_lock(&lock_paginas_para_swapeo);
    //si existia, lo saco para volverlo a poner para que no quede al principio
    if(string_equals_ignore_case(algoritmo_swap, "lru")){
        list_remove_by_condition(paginas_para_swapeo, pagina_existente_en_lista);
        list_add(paginas_para_swapeo, pagina);
        pthread_mutex_unlock(&lock_paginas_para_swapeo);
        return;
    }
    if(string_equals_ignore_case(algoritmo_swap, "clock_modificado") && op == CARGA){
        list_remove_by_condition(paginas_para_swapeo, pagina_existente_en_lista);
        list_add(paginas_para_swapeo, pagina);
        pthread_mutex_unlock(&lock_paginas_para_swapeo);
        return;
    }
    pthread_mutex_unlock(&lock_paginas_para_swapeo);
    return;
}

bool esta_unlockeada(Info_Pagina_Plato *pag){
        return pag->bit_de_lockeo == 0;
}

Info_Pagina_Plato *seleccion_de_victima_lru(){
    return list_remove_by_condition(paginas_para_swapeo,esta_unlockeada);
}

bool es_0_0(Info_Pagina_Plato *pagina){
    return (pagina->bit_de_uso == 0 && pagina->bit_de_modificacion == 0 && pagina->bit_de_lockeo == 0);
}

bool es_0_1(Info_Pagina_Plato *pagina){
    if(pagina->bit_de_lockeo == 1) printf("fail\n");
    if(pagina->bit_de_lockeo == 1) return false;
    if(pagina->bit_de_uso == 1){
        pagina->bit_de_uso = 0;
        return false;
    }
    return true;
}

int indice_siguiente_lista_circular(int indice_actual){
    if((indice_actual+1) >= list_size(paginas_para_swapeo)){
        return 0;
    }else
    {
        indice_actual++;
        return indice_actual;
    } 
}

Info_Pagina_Plato *circular_list_get_by_condition(Info_Pagina_Plato *pagina_requerida, bool (*condicion)(void*)){
    int index_actual = index_puntero_cm;
    Info_Pagina_Plato *target;
    for (int i = 0; i < list_size(paginas_para_swapeo); i++)
    {
        target = list_get(paginas_para_swapeo, index_actual);
        if(condicion(target)){
            list_replace(paginas_para_swapeo, index_actual, pagina_requerida);
            index_puntero_cm = indice_siguiente_lista_circular(index_actual);
            return target;
        }
        index_actual = indice_siguiente_lista_circular(index_actual);
    }
    return NULL; 
}

Info_Pagina_Plato *seleccion_de_victima_clock_modificado(Info_Pagina_Plato *pagina_requerida){
    Info_Pagina_Plato *target;

    if( (target = circular_list_get_by_condition(pagina_requerida, es_0_0)) ) return target;
    if( (target = circular_list_get_by_condition(pagina_requerida, es_0_1)) ) return target;
    if( (target = circular_list_get_by_condition(pagina_requerida, es_0_0)) ) return target;
    if( (target = circular_list_get_by_condition(pagina_requerida, es_0_1)) ) return target;
    return NULL;
}

int swap(Info_Pagina_Plato *pagina_requerida){

    log_info(logger, "\n\nComenzando swapeo...");
    Pagina *frame_requerido = obtener_plato_de_memoria(memoria_virtual, pagina_requerida->nro_de_frame_memoria_virtual);

    Info_Pagina_Plato *pagina_target;
    if(string_equals_ignore_case(algoritmo_swap,"lru")){
        pthread_mutex_lock(&lock_paginas_para_swapeo);
        pagina_target = seleccion_de_victima_lru();
        if(pagina_target == NULL) log_error(logger, "COMANDA: a punto de romper: no saco a ninguno en la seleccion de victima\n");
        pthread_mutex_unlock(&lock_paginas_para_swapeo);
        agregar_a_lista_de_swapeo(pagina_requerida, CARGA);
    }else
    {
        pthread_mutex_lock(&lock_paginas_para_swapeo);
        pagina_target = seleccion_de_victima_clock_modificado(pagina_requerida);
        if(pagina_target == NULL) log_error(logger, "COMANDA: a punto de romper: no saco a ninguno en la seleccion de victima\n");
        pthread_mutex_unlock(&lock_paginas_para_swapeo);
    }
    int frame_mp_target = pagina_target->nro_de_frame_memoria_principal;
    
    Pagina *aux = obtener_plato_de_memoria(memoria_principal, frame_mp_target);
    log_info(logger, "Victima seleccionada: Nombre de plato %s, Numero de frame target: %i", aux->nombre_plato, frame_mp_target);
    if(pagina_target->bit_de_modificacion == 1){
        log_info(logger, "Sincronizando con la memoria virtual...");
        Pagina *frame_target = obtener_plato_de_memoria(memoria_principal, frame_mp_target);
        guardar_plato_en_memoria(memoria_virtual, pagina_target->nro_de_frame_memoria_virtual, frame_target);
        log_info(logger, "Sincronizacion finalizada!");
        free(frame_target->nombre_plato);
        free(frame_target);
    }
    guardar_plato_en_memoria(memoria_principal, frame_mp_target, frame_requerido);
    pagina_levantada_en_memoria_principal(pagina_requerida, frame_mp_target);

    pagina_target->bit_de_presencia = 0;
    pagina_target->nro_de_frame_memoria_principal = -1;
    //porque ya no estará en la lista de swapeo, por lo tanto el iterador no lo va a unlockear
    pthread_mutex_unlock(&(pagina_target->lock_pagina));
    cantidad_de_swapeos++;
    log_info(logger, "Swapeo %i finalizado.\n\n", cantidad_de_swapeos);

    free(frame_requerido->nombre_plato);
    free(frame_requerido);
    free(aux->nombre_plato);
    free(aux);
    return 0;
}

void lockear_paginas_en_memoria(){
    list_iterate(paginas_para_swapeo, lockear_pagina);
}

void unlockear_paginas_en_memoria(){
    list_iterate(paginas_para_swapeo, unlockear_pagina);
}

//Para testear el estado actual de las memorias//
void leer_plato_de_memoria(void *memoria, int frame){
    int frame_objetivo = TAMANIO_PAGINA * frame;
    int n1;
    int n2;
    char *plato = (char *) malloc(TAMANIO_NOMBRE_PLATO);
    int desplazamiento = 0;
    memcpy(&n1, memoria + frame_objetivo + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&n2, memoria + frame_objetivo + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(plato, memoria + frame_objetivo + desplazamiento, TAMANIO_NOMBRE_PLATO);
    log_info(logger, "COMANDA: Nombre del plato: %s ; Cant_total: %i ; Cant_lista: %i", plato, n1, n2);
    free(plato);
}

void mostrar_pagina(Info_Pagina_Plato *pagina){
    printf("Mostrando Pagina\n");
    printf("Nro de frame memoria principal: %i\n", pagina->nro_de_frame_memoria_principal);
    printf("Nro de frame memoria virtual:   %i\n", pagina->nro_de_frame_memoria_virtual);
    printf("Bit de Uso:                     %i\n", pagina->bit_de_uso);
    printf("Bit de presencia:               %i\n", pagina->bit_de_presencia);
    printf("Bit de modificacion:            %i\n", pagina->bit_de_modificacion);
    leer_plato_de_memoria(memoria_virtual, pagina->nro_de_frame_memoria_virtual);
    printf("Fin\n\n");
}

void mostrar_memoria_principal(){
    printf("Mostrando elementos cargandos en la MEMORIA PRINCIPAL\n");
    for(int i = 0; i<cantidad_de_frames_mp; i++){
        if(bitmap_memoria_principal[i] == 1){
            printf("Pagina %i\n", i);
            leer_plato_de_memoria(memoria_principal, i);
        }
    }
    printf("Fin\n\n");
}

void mostrar_memoria_virtual(){
    printf("Mostrando elementos cargandos en la MEMORIA VIRTUAL\n");
    for(int i = 0; i<cantidad_de_frames_mv; i++){
        if(bitmap_memoria_virtual[i] == 1){
            printf("Pagina %i\n", i);
            leer_plato_de_memoria(memoria_virtual, i);
        }
    }
    printf("Fin\n\n");
}

//-----------------//
void free_palabras(char **palabras, int size) {
    for (int i = 0; i < size; i++)
        free(palabras[i]);

    free(palabras);

    return;
}

void catch_signal(int signal) {
    terminar_programa();
}

void liberar_info_pag(Info_Pagina_Plato *pag){
    pthread_mutex_destroy(&(pag->lock_pagina));
    free(pag);
}

void destruir_pedido(Segmento_Pedido *pedido){
    dictionary_destroy_and_destroy_elements(pedido->tabla_de_platos,liberar_info_pag);
    free(pedido);
}

void destruir_restaurante(t_dictionary *tabla_de_pedidos){
    dictionary_destroy_and_destroy_elements(tabla_de_pedidos,destruir_pedido);
}

int terminar_programa() {
    log_info(logger, "COMANDA: Exit");
    if(socket_server > 0) liberar_conexion(socket_server);
    free(memoria_principal);
    free(memoria_virtual);
    free(bitmap_memoria_principal);
    free(bitmap_memoria_virtual);
    dictionary_destroy_and_destroy_elements(tablas_de_restaurantes, destruir_restaurante);
    list_destroy(paginas_para_swapeo);
    pthread_mutex_destroy(&lock_bitmap_mp);
    pthread_mutex_destroy(&lock_bitmap_mv);
    pthread_mutex_destroy(&lock_swap);
    pthread_mutex_destroy(&lock_paginas_para_swapeo);
    log_destroy(logger);
    config_destroy(config);
    exit(0);
}