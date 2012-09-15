all:   red log configuracion client
	gcc -Wall server.c funciones_red.o configuracion.o log.o -lpthread -o server
clean:  
	rm *.o
red:	
	gcc -Wall -c funciones_red.c
log:   
	gcc -Wall -c log.c
configuracion:
	gcc -Wall -c configuracion.c
client:
	gcc -Wall client.c funciones_red.o configuracion.o log.o -o client
