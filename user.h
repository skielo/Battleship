#include <stdlib.h>

#define LOG_MENSAJE_EXTRA -1
#define LOG_CONEXION_CLIENTE 1
#define LOG_COMANDO_CLIENTE 2
#define LOG_ERROR 3
#define LOG_CIERRE_SOCKET_LISTEN 4
#define LOG_CIERRE_SOCKET_ACCEPT 5
#define LOG_JUGADA_ENVIADA 6
#define LOG_JUGADA_RECIVIDA 7
#define LOG_CONEXION_SERVER 1

#define NUMBER_X 10
#define NUMBER_Y 10
#define MAXLENGHT 25

/*Estructura de un Cliente*/
typedef struct Client{
  char sNombre[MAXLENGHT];
	char iBoatTable[NUMBER_X][NUMBER_Y];		
	char iPlayTable[NUMBER_X][NUMBER_Y];	
}stClient;

/*Estructura que representa el header de los mensajes*/
typedef struct Header{
	char sMensaje[MAXLENGHT];
	int iCantidad;
	char sNombre[MAXLENGHT];
	int iFinPartida;
}stHeader;

/*Estructura de la lista de Clientes*/
typedef struct sNodoClient{
  int iSock;
	/*Direccion y puerto donde deben conectarse con el servidor que atiende al cliente*/
  char sDireccionIP[MAXLENGHT];
	int sPuerto;
	int iJugando;
	/*Agrego la info del cliente en esta misma estructura*/
  char sNombre[MAXLENGHT];
	char iBoatTable[NUMBER_X][NUMBER_Y];		
	char iPlayTable[NUMBER_X][NUMBER_Y];	
  struct sNodoClient * sig;
}NODOClient;

typedef NODOClient * LISTACLIENT;
typedef NODOClient * ListasClient;

stClient MakeClient(char *, char[],  FILE *);
int ReadClient(FILE *, stClient * ,LISTACLIENT * , int );
void matrix_init(char [],stClient*);
