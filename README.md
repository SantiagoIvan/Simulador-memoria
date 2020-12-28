# Simulador-memoria
Simulador del funcionamiento de memoria con swap, utilizando los algoritmos Clock modificado y LRU.

Se pueden enviar los siguientes mensajes desde el proceso Cliente al proceso Comanda:
- guardar_pedido <restaurante> <nro de pedido>
- guardar_plato <restaurante>  <nro de pedido> <nombre de plato> <cantidad>
- obtener_pedido <restaurante> <nro de pedido>
- confirmar_pedido <restaurante> <nro de pedido>
- plato_listo <restaurante> <nro de pedido> <nombre de plato>: se van preparando de a 1, y cuando esten todos listos, es decir, cantidad pedida= cantidad preparada,
recien ahi el mensaje finalizar_pedido podrá ser enviado exitósamente.
- finalizar_pedido <restaurante> <nro de pedido>
