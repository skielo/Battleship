all:  red log configuracion user print client
	gcc -ggdb -g -fno-stack-protector -Wall server.c funciones_red.o configuracion.o log.o user.o funciones_print.o -lpthread -o server
clean:  
	rm *.o server client
red:	
	gcc -Wall -c funciones_red.c
log:   
	gcc -Wall -c log.c
configuracion:
	gcc -Wall -c configuracion.c
user:
	gcc -Wall -c user.c
print:
	gcc -Wall -c funciones_print.c
client:
	gcc -ggdb -g -Wall client.c funciones_red.o configuracion.o log.o user.o funciones_print.o -o client
