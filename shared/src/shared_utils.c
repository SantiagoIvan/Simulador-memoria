#include "shared_utils.h"

int crear_conexion(char *ip, char *puerto) {
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &server_info);

    int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        printf("Error al conectarse\n");
        freeaddrinfo(server_info);
        return -1;
    }

    freeaddrinfo(server_info);

    return socket_cliente;
}

void liberar_conexion(int socket_cliente) {
    close(socket_cliente);
}

Mensaje *crear_mensaje(Codigo_Operacion codigo_operacion) {
    Mensaje *mensaje = malloc(sizeof(Mensaje));
    mensaje->codigo_operacion = codigo_operacion;
    crear_payload(mensaje);
    return mensaje;
}

void crear_payload(Mensaje* mensaje) {
    mensaje->payload = malloc(sizeof(Payload));
    mensaje->payload->size = 0;
    mensaje->payload->stream = NULL;
}

void enviar_mensaje(Mensaje *mensaje, int socket_cliente) {
    int bytes = mensaje->payload->size + 2 * sizeof(int);
    void *a_enviar = serializar_mensaje(mensaje, bytes);

    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar);
    free(mensaje->payload->stream);
    free(mensaje->payload);
    free(mensaje);
}

void* serializar_mensaje(Mensaje *mensaje, int bytes) {
    void* magic = malloc(bytes);
    int desplazamiento = 0;

    memcpy(magic + desplazamiento, &(mensaje->codigo_operacion), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, &(mensaje->payload->size), sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(magic + desplazamiento, mensaje->payload->stream, mensaje->payload->size);
    desplazamiento += mensaje->payload->size;

    return magic;
}

Codigo_Operacion get_codigo_operacion_by_string(char *string_codigo_operacion) {
    if (strcmp(string_codigo_operacion, "consultar_restaurantes") == 0) return CONSULTAR_RESTAURANTES;
    if (strcmp(string_codigo_operacion, "seleccionar_restaurante") == 0) return SELECCIONAR_RESTAURANTE;
    if (strcmp(string_codigo_operacion, "obtener_restaurante") == 0) return OBTENER_RESTAURANTE;
    if (strcmp(string_codigo_operacion, "consultar_platos") == 0) return CONSULTAR_PLATOS;
    if (strcmp(string_codigo_operacion, "crear_pedido") == 0) return CREAR_PEDIDO;
    if (strcmp(string_codigo_operacion, "guardar_pedido") == 0) return GUARDAR_PEDIDO;
    if (strcmp(string_codigo_operacion, "aniadir_plato") == 0) return ANIADIR_PLATO;
    if (strcmp(string_codigo_operacion, "guardar_plato") == 0) return GUARDAR_PLATO;
    if (strcmp(string_codigo_operacion, "confirmar_pedido") == 0) return CONFIRMAR_PEDIDO;
    if (strcmp(string_codigo_operacion, "plato_listo") == 0)return PLATO_LISTO;
    if (strcmp(string_codigo_operacion, "consultar_pedido") == 0) return CONSULTAR_PEDIDO;
    if (strcmp(string_codigo_operacion, "obtener_pedido") == 0) return OBTENER_PEDIDO;
    if (strcmp(string_codigo_operacion, "finalizar_pedido") == 0) return FINALIZAR_PEDIDO;
    if (strcmp(string_codigo_operacion, "terminar_pedido") == 0) return TERMINAR_PEDIDO;
    if (strcmp(string_codigo_operacion, "obtener_receta") == 0) return OBTENER_RECETA;
    if (strcmp(string_codigo_operacion, "crear_restaurante") == 0) return CREAR_RESTAURANTE;
    if (strcmp(string_codigo_operacion, "crear_receta") == 0) return CREAR_RECETA;
    if (strcmp(string_codigo_operacion, "prueba") == 0) return PRUEBA;

    if (strcmp(string_codigo_operacion, "exit") == 0) return SALIR;
    if (strcmp(string_codigo_operacion, "connect") == 0) return CONNECT;
    if (strcmp(string_codigo_operacion, "prueba_final_comanda") == 0) return PRUEBA_FINAL_COMANDA;
    
}

void nuevo_cliente_conectado(t_list *lista,int conexion, Cliente_Conectado *cliente){

    char *ip = obtener_ip_conexion(conexion);
    int size = strlen(ip);
    cliente->ip = (char *) malloc(size);
    strcpy(cliente->ip, ip);
    list_add(lista, cliente);
    return;
}

void liberar_cliente(Cliente_Conectado *cliente){
    free(cliente);
    return;
}

void liberar_clientes(t_list *lista){

    list_destroy_and_destroy_elements(lista, liberar_cliente);
    return;
}

void nuevo_restaurante_conectado(t_list *lista,int conexion, Restaurante_Conectado *rest){

    char *ip = obtener_ip_conexion(conexion);
    int size = strlen(ip);
    rest->ip = (char *) malloc(size);
    strcpy(rest->ip, ip);
    list_add(lista, rest);
    return;
}

void liberar_restaurante(Restaurante_Conectado *rest){
    free(rest->nombre);
    free(rest);
    return;
}

void liberar_restaurantes(t_list *lista){
    list_destroy_and_destroy_elements(lista, liberar_restaurante);
    return;
}

void mostrar_cliente(Cliente_Conectado *cliente){
    printf("Cliente ID: %i\n", cliente->id);
    printf("Posicion X: %i\n", cliente->posicion.x);
    printf("Posicion Y: %i\n", cliente->posicion.y);
    printf("IP: %s\n",cliente->ip);
}

void mostrar_clientes(t_list *lista){

    printf("Mostrando clientes...\n");
    list_iterate(lista, mostrar_cliente);
    printf("Todos los clientes mostrados!\n");
    return;
}

void mostrar_restaurante(Restaurante_Conectado *rest){
    printf("Restaurante %s \n",rest->nombre);
    printf("Posicion x %i \n",rest->posicion.x);
    printf("Posicion y %i \n",rest->posicion.y);
    printf("IP: %s\n",rest->ip);
}

void mostrar_restaurantes(t_list *lista){

    printf("Mostrando restaurantes...\n");
    list_iterate(lista, mostrar_restaurante);
    printf("Todos los restaurantes mostrados!\n");
    return;
}

char *obtener_ip_conexion(int conexion) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    if ( getpeername(conexion, (struct sockaddr *)&addr, &addr_size) < 0 ) return NULL;
    char *ip = malloc(20);
    strcpy(ip, inet_ntoa(addr.sin_addr));
    return ip;
}

t_list* get_list_from_array(char** values) {
	t_list* list = list_create();

	void get(char* string) {
		if (string != NULL) list_add(list, string);
	}

	string_iterate_lines(values, get);
	free(values);

	return list;
}