/*Estructura de un Cliente*/
typedef struct Client{
  unsigned char ucPort;
  int ucIPaddress;
	int iBoatTable[10][10];		
	int iPlayTable[10][10];	
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

stClient MakeClient(int , char *, int[10][10],  FILE *);
int ReadClient(FILE *, stClient * ,LISTACLIENT * , int );
