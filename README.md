# Simulador-memoria
Simulador del funcionamiento de memoria con swap, utilizando los algoritmos Clock modificado y LRU.

Consiste en un modulo del Trabajo practico de la materia Sistemas Operativos de la UTN FRBA, que simula el almacenamiento de pedidos junto con sus platos, en paginas de una memoria.

Cada restaurante será representado con una tabla de Segmentos, donde cada segmento representará a un Pedido, y cada página representará a un plato.

Está dividido en 2 procesos:

Cliente: oficia de consola y por medio de una conexión via sockets, envía mensajes a la Memoria llamada Comanda.

Comanda: proceso que simula el funcionamiento de una memoria con Segmentación Paginada, y además simula el funcionamiento de una memoria virtual y sus procesos de detección de víctima y swapeo. El algoritmo de selección de víctima puede ser Clock modificado o LRU y son modificables desde el config ubicado en comanda/cfg/comanda.config

Su interfaz es la siguiente:
- **guardar_pedido** ( restaurante , nro_de_pedido ): agrega un nuevo pedido al restaurante. La memoria por su parte creará una tabla de segmentos para dicho restaurante si no existe, y un segmento para ese pedido.
- **guardar_plato** ( restaurante  , nro_de_pedido , nombre_de_plato , cantidad ): Guarda un nuevo plato, agregando una pagina al segmento correspondiente al pedido
- **obtener_pedido** ( restaurante , nro_de_pedido ): se obtiene la información de un pedido en cualquier momento de su ciclo de vida.
- **confirmar_pedido** ( restaurante , nro_de_pedido ): se confirma un pedido, cerrando asi la posibilidad de agregar nuevos platos y comenzando el proceso de preparación. A partir de este momento se podrán enviar el mensaje plato_listo.
- **plato_listo** ( restaurante , nro_de_pedido , nombre_de_plato ): se van preparando de a 1, y cuando esten todos listos, es decir, cantidad pedida= cantidad preparada, finalizar_pedido podrá ser enviado exitósamente.
- **finalizar_pedido** ( restaurante , nro_de_pedido ): finaliza un pedido, liberando el segmento correspondiente en memoria y las paginas utilizadas.
- **prueba_final_comanda**: se inicia el envío de todos los mensajes utilizados en la prueba final del modulo. Consiste en iniciar varios pedidos de varios restaurantes, ir almacenando sus platos, confirmar los pedidos, preparar los platos y luego finalizar los pedidos. En el proceso, la memoria principal se irá llenando y será necesario swapear una pagina para poder levantar en memoria a la pagina requeria para la modificación.


  
Con los comandos ./exec pueden correr alguno de los procesos, ubicandose previamente en alguna de las carpetas Cliente o Comanda.
Con ./vexec pueden correr los procesos usando valgrind.
