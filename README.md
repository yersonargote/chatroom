# chatroom
Este proyecto toma como base el proyecto de 
https://github.com/nikhilroxtomar/Chatroom-in-C para implementar los 
requerimientos necesarios para solucionar el parcial de Sistemas Operativos.

### Planteamiento del problema

Se deberá desarrollar dos programas en C que implementen la funcionalidad de 
un chat simple entre dos sistemas conectados a la red, usando sockets.

Uno de los extermos actuará como cliente, y el otro como servidor. Quien recibe
conexiones (el servidor) enviará al cliente un mensaje de bienvenida, después
del cual, el cliente deberá enviar un mensaje, y el servidor deberá recibir. Este
proceso terminará cuando en el servidor o en el cliente el usuario ingrese un
mensaje con el texto /exit".
Se debe permitir la comunicación entre un servidor y múltiples clientes
clientes. Además, los clientes también podrán comunicarse entre sí, a través
del servidor.
