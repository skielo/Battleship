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

/*Estructura de un Cliente*/
typedef struct Client{
  char * sNombre;
	char iBoatTable[NUMBER_X][NUMBER_Y];		
	char iPlayTable[NUMBER_X][NUMBER_Y];	
}stClient;
/*Estructura de la lista de Conexiones*/
typedef struct sNodoListas{
  int iSock;
  char cEstado;
  int iInfo;
  struct sNodoListas * sig;
}NODOListas;
/*Estructura de la lista de Clientes*/
typedef struct sNodoClient{
  int iSock;
  stClient * sClient;
  struct sNodoClient * sig;
}NODOClient;

typedef NODOListas * LISTANODO;
typedef NODOListas * ListasNodos;
typedef NODOClient * LISTACLIENT;
typedef NODOClient * ListasClient;

stClient MakeClient(char *, char[][NUMBER_Y],  FILE *);
int ReadClient(FILE *, stClient * ,LISTACLIENT * , int );
void matrix_init(char [][NUMBER_Y],stClient*);
