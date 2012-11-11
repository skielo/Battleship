#include "log.h"
#include "configuracion.h"
#include "funciones_red.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "mensajes.h"
#include "server.h"
#include <ctype.h>
#include <pthread.h>

#define BUFSIZE 1025
#define PUERTO 10001

#define LOG_MENSAJE_EXTRA -1
#define LOG_CONEXION_CLIENTE 1
#define LOG_COMANDO_CLIENTE 2
#define LOG_ERROR 3
#define LOG_CIERRE_SOCKET_LISTEN 4
#define LOG_CIERRE_SOCKET_ACCEPT 5
#define LOG_JUGADA_ENVIADA 6
#define LOG_JUGADA_RECIVIDA 7

#ifdef MUTEX
/* mutex para sincronizar el acceso a buffer */
pthread_mutex_t mutexBuffer;
#endif
//Variable global con la lista de conexiones activdas
ListasClient lConexiones;

int main(int argc, char** argv)
{
  char* sDireccion;
  char* sPuerto;
	char sCommonAnswer[10] = "OK \r\n";
  int sockListen,sockClient,iThread;
  struct sockaddr_in addrListen, addrClient;
  socklen_t clilen;
  FILE* fConfiguracion;
  FILE* fLog;
	pthread_t idChild;
	stParam param; 
  fLog=ArchivoLog("./server.log");
  fConfiguracion=AbrirArchivo("./server.conf");
  sockListen=MakeSocket(AF_INET,SOCK_STREAM,0);
  sDireccion=malloc(80);
  sPuerto=malloc(6);
	lConexiones=malloc(sizeof(NODOClient));

  if(LeerValor(fConfiguracion,"DIRECCION",sDireccion)==1)
  {
    printf("Error al leer la direccion\n");
    exit (EXIT_FAILURE);
  }

  if(LeerValor(fConfiguracion,"PUERTO",sPuerto)==1)
  {
    printf("Error al leer el puerto\n");
    exit (EXIT_FAILURE);
  }

	//Habilito al servidor a recib ir multiples conexiones
	AllowMultipleConection(sockListen);

  bzero(&addrListen,sizeof(addrListen));
  addrListen.sin_family = AF_INET;
  addrListen.sin_port = htons(atoi(sPuerto));
  if(inet_pton(AF_INET,sDireccion,&addrListen.sin_addr) <0)
  {
    perror("inet_pton");
    exit (EXIT_FAILURE);
  }
  LimpiarCRLF(sDireccion);
  if(BindSocket(sockListen,(struct sockaddr*)&addrListen,sizeof(addrListen)) < 0)
  {
    perror("BindSocket");
    Log(LOG_MENSAJE_EXTRA,fLog,"Error Estableciendo la Conexion de Escucha\n");
    exit (EXIT_FAILURE);
  }
  if(ListenSocket(sockListen,10)<0)
  {
    perror("ListenSocket");
    Log(LOG_MENSAJE_EXTRA,fLog,"Error Escuchando En La Conexion\n");
    exit (EXIT_FAILURE);
  }

#ifdef MUTEX
	/* Se inicializa el mutex */
	pthread_mutex_init (&mutexBuffer, NULL);
#endif

	GenerarPuerto(atoi(sPuerto));
  clilen=sizeof(addrClient);
  printf("Servidor de juego V 1.0\n-----------");
  printf("\nEsperando conexiones de clientes en %s:%s\n",sDireccion,sPuerto);
	printf("--------------------------------------------------------------\n");
  while(1)
  {
		fflush(stdout);
		printf("Esperando nuevas conexiones\n");
    sockClient=AcceptSocket(sockListen,(struct sockaddr*)&addrClient,&clilen);
    Log(LOG_CONEXION_CLIENTE,fLog,"");
    printf("\nConexion entrante aceptada\n-----------------\n");

		//Enviamos la confirmacion al cliente y le decimos donde debe conectarse
    WriteSocket(sockClient,sCommonAnswer,strlen(sCommonAnswer),0);
		Log(LOG_MENSAJE_EXTRA,fLog,"Se envio al cliente la respuesta comun");
		param=ArmarParametros(sockClient,sDireccion,sPuerto,fLog);
		
		iThread = pthread_create(&idChild, NULL, ConexionControl, &param);
		Log(LOG_MENSAJE_EXTRA,fLog,"Se inicializo el thread y ahora se esperan mas conexiones");
    if(iThread != 0)
    {
			perror("THREAD");
      fclose(fConfiguracion);
      fclose(fLog);
      CloseSocket(sockListen);
      break;
    }
  }
  fclose(fLog);
  return EXIT_SUCCESS;
}

void ComandoInvalido(int sockClient)
{
	WriteSocket(sockClient,sMsg_ComandoInvalido,strlen(sMsg_ComandoInvalido),0);
}

/*Limpia el CRLF*/
void LimpiarCRLF(char *sCadena)
{
	char *aux;
	aux=strchr(sCadena,'\r');
	if(aux==NULL)
	 aux=strchr(sCadena,'\n');
	if(aux != NULL)
	 aux[0]='\0';
}


/*Cambia una string a mayuscula*/
void Mayusculas(char* sCadena)
{
 int x;
 for(x=0;x<strlen(sCadena);x++)
  sCadena[x]=toupper(sCadena[x]);
}

/*Genera un nuevo puerto para el cliente que ingreso*/
int GenerarPuerto(int iPuerto)
{
	static int Puerto;

	if(iPuerto >0)
	{
		Puerto=iPuerto;
		return 0;
	}
	else
	{
		Puerto++;
		return Puerto;
	}
}


/*Arma los parametros para iniciar el nuevo thread*/
stParam ArmarParametros(int sockClient,char* sDireccionIP,char* sPuerto, FILE * fLog)
{
	//printf("socket: %d, ip: %s, puerto: %s\n",sockClient, sDireccionIP, sPuerto);
	stParam retval;
	retval.sockClient=sockClient;
  memset(retval.sPuerto,(unsigned char)sPuerto, 1);
	retval.sDireccionIP = malloc(sizeof(sDireccionIP));
  strcpy(retval.sDireccionIP, sDireccionIP);
	retval.fLog = fLog;
	return retval;
}


/*Esta funcion se encarga de manejar la conexion con el cliente, esperando
comandos desde el mismo y evaluandolos*/
void * ConexionControl(void * param)
{
	stParam * p = (stParam *)param;
	int bytesRead, i;
	char buf[BUFSIZE];
	char* command;
	int codigoComando, sockControl;
	stClient cliente;
	fd_set fdControl;
	struct timeval tv;

	//Inicio la conexion de control con el cliente
	//sockControl = CrearConexionDeControl(p->sDireccionIP,p->sPuerto);
	//Recibo los datos que me va a mandar el cliente sobre el mismo
	bytesRead=ReadSocket(p->sockClient,&cliente,sizeof(stClient),0);
  if(bytesRead <0)
  {
    perror("ReadSocket");
    exit (EXIT_FAILURE);
	}

#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif

	//Insert el nuevo cliente en la lista de conexiones compartida		
	InsertarListaConexion(p->sockClient, &cliente, lConexiones);

#ifdef MUTEX
	/* Se desbloquea el mutex */
	pthread_mutex_unlock (&mutexBuffer);
#endif

  command=malloc(5);
  while(1)
  {
		fflush(stdout);
    bzero(buf,BUFSIZE);
		Log(LOG_MENSAJE_EXTRA,p->fLog,"Esperando mensajes del cliente");
    bytesRead=ReadSocket(p->sockClient,buf,BUFSIZE,0);
    if(bytesRead >0)
    {
      LimpiarCRLF(buf);
      command=strtok(buf," ");
			printf("Comando: %s \n", command);
      codigoComando=EvaluarComando(command);
      switch(codigoComando)
      {
        case 1: /*LIST*/
              continue;
        case 2: /*GAME*/
              continue;
				case 3: /*PLAY*/
							continue;
        case 4: /*QUIT*/
              Quit(p->sockClient); /*Salir directamente*/
              return 0;
        default: /*Comando invalido*/
              ComandoInvalido(p->sockClient);
              break;
      }
    }
  }
}

/*Evalua el comando recibido para actuar
en consecuencia*/
int EvaluarComando(char* sComando)
{
  Mayusculas(sComando);
  if(strcmp(sComando,"LIST")==0)
    return 1;

  if(strcmp(sComando,"GAME")==0)
    return 2;

  if(strcmp(sComando,"PLAY")==0)
    return 3;

  if(strcmp(sComando,"QUIT")==0)
    return 4;

  return -1;
}

/*Cierra la conexion y envia un mensaje al cliente para
despedirse*/
void Quit(int sockClient)
{
  FILE* fLog;
  fLog=ArchivoLog("./ftpserver.log");
  WriteSocket(sockClient,sMsg_Quit,strlen(sMsg_Quit),0);
  Log(LOG_CIERRE_SOCKET_ACCEPT,fLog,"");
  fclose(fLog);
}

/* Objetivo: Esta funcion, arma el nodo del tipo sAuxLista lo inserta en 1era posicion y 
devuelve el puntero a ListaConexion */
int InsertarListaConexion( int ifdsock, stClient * sClient, LISTACLIENT *Lista)
{
    LISTACLIENT sAux;

    sAux=malloc(sizeof(LISTACLIENT));
    if(sAux== NULL) 
       return -1;         /*Retorno -1 si hubo problemas con la memoria*/
	   
    /*Lleno la estructura con los datos*/   
    sAux->iSock = ifdsock;
    sAux->sClient = sClient;
    sAux->sig = *Lista;

    *Lista = sAux;
    
    return 1;               /*Retorno 1 si pude insertarlo bien*/
}

/*Crea la conexion de control donde el cliente va a recibir mensajes desde el 
servidor y los otros threads van a comunicarse*/
int CrearConexionDeControl(char * direccion, char * puerto)
{
	int socketRet = MakeSocket(AF_INET,SOCK_STREAM,0);;
  struct sockaddr_in addrListen;
  socklen_t clilen;
	
	AllowMultipleConection(socketRet);

  bzero(&addrListen,sizeof(addrListen));
  addrListen.sin_family = AF_INET;
  addrListen.sin_port = htons(GenerarPuerto(atoi(puerto)));

  if(inet_pton(AF_INET,direccion,&addrListen.sin_addr) <0)
  {
    perror("inet_pton");
    exit (EXIT_FAILURE);
  }
  LimpiarCRLF(direccion);
  if(BindSocket(socketRet,(struct sockaddr*)&addrListen,sizeof(addrListen)) < 0)
  {
    perror("BindSocket");
    exit (EXIT_FAILURE);
  }
  if(ListenSocket(socketRet,10)<0)
  {
    perror("ListenSocket");
    exit (EXIT_FAILURE);
  }	

	return socketRet;
}
