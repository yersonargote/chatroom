Elaborado por: Erwin Meza Vega <emezav@unicauca.edu.co>
Modificado por: Yerson Argote <yersonargote>

Implementacion de chat entre un servidor y multiples clientes.

Uso del servidor:

./server [puerto]

Inicia el servidor en el puerto especificado.
Si el usuario no especifica el puerto, se toma
por defecto el puerto número 6530.

NOTA: El servidor debe ejecutarse primero, de forma
que los clientes puedan conectarse.

Posiblemente el servidor debera ser instalado como
un servicio del sistema que es arrancado automaticamente
cuando inicia el sistema.


Uso del cliente:

./client IP_SERVER [puerto]

Inicia una conexión de chat al servidor ubicado en la
dirección IP_SERVER, en el puerto especificado. Si no se
especifica, la conexión se realizará al puerto 1234.
