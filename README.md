# Simulador-memoria
Simulador del funcionamiento de memoria con swap, utilizando los algoritmos Clock modificado y LRU.

Consiste en un modulo del Trabajo practico de la materia Sistemas Operativos de la UTN FRBA, que simula el almacenamiento de pedidos junto con sus platos, en paginas de una memoria.

Está dividido en 2 procesos:

Cliente: oficia de consola y por medio de una conexión via sockets, envía mensajes a la Memoria llamada Comanda.
- guardar_pedido <restaurante> <nro de pedido>
- guardar_plato <restaurante>  <nro de pedido> <nombre de plato> <cantidad>
- obtener_pedido <restaurante> <nro de pedido>
- confirmar_pedido <restaurante> <nro de pedido>
- plato_listo <restaurante> <nro de pedido> <nombre de plato>: se van preparando de a 1, y cuando esten todos listos, es decir, cantidad pedida= cantidad preparada,
recien ahi el mensaje finalizar_pedido podrá ser enviado exitósamente.
- finalizar_pedido <restaurante> <nro de pedido>

Comanda: proceso que simula el funcionamiento de una memoria con Segmentación Paginada, y además simula el funcionamiento de una memoria virtual y sus procesos de detección de víctima y swapeo. El algoritmo de selección de víctima puede ser Clock modificado o LRU y son modificables desde el config ubicado en comanda/cfg/comanda.config
