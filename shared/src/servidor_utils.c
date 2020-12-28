#include"servidor_utils.h"

int iniciar_servidor(char* ip,char* puerto)
{
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_payload(int* size, int socket_cliente)
{
	void * payload;
	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	payload = malloc(*size);
	recv(socket_cliente, payload, *size, MSG_WAITALL);
	return payload;
}

//podemos usar la lista de valores para poder hablar del for y de como recorrer la lista
t_list* recibir_mensaje(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * payload;
	t_list* valores = list_create();
	int tamanio;

	payload = recibir_payload(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, payload + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, payload+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(payload);
	return valores;
	return NULL;
}

void enviar_identificacion(int conexion, Codigo_Operacion id){
	Mensaje *mensaje = crear_mensaje(id);
	enviar_mensaje(mensaje, conexion);
}

void enviar_identificacion_de_cliente(int conexion, int id, int x, int y){
	Mensaje *mensaje = crear_mensaje(CLIENTE);
	mensaje->payload->size = 3*sizeof(int);
	mensaje->payload->stream = malloc(mensaje->payload->size);
	
	int desplazamiento = 0;

	serializar_int(mensaje->payload->stream, id, &desplazamiento);
	serializar_int(mensaje->payload->stream, x, &desplazamiento);
	serializar_int(mensaje->payload->stream, y, &desplazamiento);

	enviar_mensaje(mensaje, conexion);
}

Cliente_Conectado *recibir_identificacion_de_cliente(int conexion, void *stream){
	
	Cliente_Conectado *cliente = (Cliente_Conectado *) malloc(sizeof(Cliente_Conectado));
	int desplazamiento = 0;

	deserializar_int(&(cliente->id), stream, &desplazamiento);
	deserializar_int(&(cliente->posicion.x), stream, &desplazamiento);
	deserializar_int(&(cliente->posicion.y), stream, &desplazamiento);

	return cliente;
}

void enviar_identificacion_de_restaurante(int conexion, int size_nombre, char *nombre, int x, int y){
	Mensaje *mensaje = crear_mensaje(RESTAURANTE);
	mensaje->payload->size = 3*sizeof(int) + size_nombre + 1;
	mensaje->payload->stream = malloc(mensaje->payload->size);
	int desplazamiento = 0;

	serializar_string(mensaje->payload->stream, size_nombre, nombre, &desplazamiento);
	serializar_int(mensaje->payload->stream, x, &desplazamiento);
	serializar_int(mensaje->payload->stream, y, &desplazamiento);

	enviar_mensaje(mensaje, conexion);
}

Restaurante_Conectado *recibir_identificacion_de_restaurante(int conexion, void *stream){
	
	Restaurante_Conectado *rest = (Restaurante_Conectado *) malloc(sizeof(Restaurante_Conectado));
	int desplazamiento = 0;
	
	deserializar_string(&(rest->size_nombre), &(rest->nombre), stream, &desplazamiento);
	deserializar_int(&(rest->posicion.x), stream, &desplazamiento);
	deserializar_int(&(rest->posicion.y), stream, &desplazamiento);

	return rest;
}

Codigo_Operacion *recibir_identificacion(int conexion, Codigo_Operacion *buffer){
    recv(conexion,buffer,sizeof(Codigo_Operacion),MSG_WAITALL);
    return buffer;
}

int handshake(char *ip, char *puerto){
	int fd;
    if( (fd = crear_conexion(ip, puerto)) > 0 ){
        Mensaje *msg = crear_mensaje(HANDSHAKE);
        enviar_mensaje(msg, fd);
        Codigo_Operacion cod = recibir_operacion(fd);
        if( cod == HANDSHAKE){
        	liberar_conexion(fd);
			return 1;
		}
    }else
	{
		return -1;
	}
	
}

void atender_conexion(int conexion_entrante, void *(*f)(void*)){
	pthread_t listener;
	pthread_attr_t atributo_hilo;
	pthread_attr_init(&atributo_hilo);
	pthread_attr_setdetachstate(&atributo_hilo,PTHREAD_CREATE_DETACHED);
    int *conexion = (int *) malloc(sizeof(int));
    *conexion = conexion_entrante;
    pthread_create(&listener, &atributo_hilo, f, conexion);
}