#include "log.h"
#include "configuracion.h"
#include "funciones_red.h"
#include "funciones_print.h"
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
#define MAXLENGHT 25

#ifdef MUTEX
/* mutex para sincronizar el acceso a buffer */
pthread_mutex_t mutexBuffer;
#endif
//Variable global con la lista de conexiones activdas
LISTACLIENT lConexiones=NULL;

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
/*
		printf("-----------------Los conectados hasta el momento-----------------\n");
		MostrarLista(lConexiones);
*/
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

int maximoValor(int lista[])
{
	int maximo=0,max_clients = 3, i;

	for (i = 0; i < max_clients; i++) 
	{
		if(maximo<lista[i])
			maximo=lista[i];
	}
	return maximo;
}


/*Arma los parametros para iniciar el nuevo thread*/
stParam ArmarParametros(int sockClient,char* sDireccionIP,char* sPuerto, FILE * fLog)
{
/*
	printf("socket: %d, ip: %s, puerto: %s\n",sockClient, sDireccionIP, sPuerto);
	printf("socket: %d, ip: %s, puerto: %s\n",retval.sockClient, retval.sDireccionIP, retval.sPuerto);
*/	
	stParam retval;
	retval.sockClient=sockClient;
	retval.sPuerto = malloc(sizeof(sPuerto));
	strcpy(retval.sPuerto,sPuerto);
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
	NODOClient * cliente;
	fd_set fdControl;
	struct timeval tv;
	//Variables para la conexion de control
	int master_socket , new_socket , client_socket[3] , max_clients = 3 , s, newPort=0,bytesRead, i, newSock=0;
	struct sockaddr_in address;
	socklen_t addrlen;
	char newDireccion[MAXLENGHT];
	
	cliente=(NODOClient *)malloc(sizeof(NODOClient));
	//Recibo los datos que me va a mandar el cliente sobre el mismo
	bytesRead=ReadSocket(p->sockClient,cliente,sizeof(NODOClient),0);

  if(bytesRead <0)
  {
    perror("ReadSocket");
    exit (EXIT_FAILURE);
	}
	Log(LOG_MENSAJE_EXTRA,p->fLog,"Recibo los datos que me va a mandar el cliente sobre el mismo\n");
	printf("Se conecto: %s en el socket: %d\n",cliente->sNombre, p->sockClient);
	//Genero el nuevo puerto donde deben conectarse
	newPort = GenerarPuerto(-1);
	//printf("conectando en %s:%d\n", p->sDireccionIP,newPort);
	fflush(stdout);
#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif

	//strcpy(newDireccion,p->sDireccionIP);
	//memset(newDireccion, (void *)p->sDireccionIP, (int)strlen(p->sDireccionIP));
	newSock=p->sockClient;
	//Insert el nuevo cliente en la lista de conexiones compartida
	memcpy(newDireccion,p->sDireccionIP,strlen(p->sDireccionIP));
	if(InsertarListaConexion(newSock, cliente, &lConexiones, newPort,newDireccion) < 0)
	{
    perror("Lista de conexion");
    exit (EXIT_FAILURE);
	}
	MostrarLista(lConexiones);

#ifdef MUTEX
	/* Se desbloquea el mutex */
	pthread_mutex_unlock (&mutexBuffer);
#endif
	Log(LOG_MENSAJE_EXTRA,p->fLog,"Insert el nuevo cliente en la lista de conexiones compartida\n");
	Log(LOG_MENSAJE_EXTRA,p->fLog,"Inicio la conexion de control para comunciarme con el cliente y con otros server que quieran jugar con este cliente\n");


	/*Inicio la conexion de control para comunciarme con el cliente y con 
	otros server que quieran jugar con este cliente*/
	
	//inicializamos todos client_socket[] en 0
	for (i = 0; i < max_clients; i++) 
	{
		client_socket[i] = 0;
	}
	//Agrego al cliente que ya esta conectado
	client_socket[0]=p->sockClient;
	/*printf("La lista quedo asi: \n");
	for (i = 0; i < max_clients; i++) 
	{
		printf("%d \n",client_socket[i]);
	}*/
	master_socket=MakeSocket(AF_INET,SOCK_STREAM,0);
	//AllowMultipleConection(master_socket);
  bzero(&address,sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(newPort);

  if(inet_pton(AF_INET,p->sDireccionIP,&address.sin_addr) <0)
  {
    perror("inet_pton");
    exit (EXIT_FAILURE);
  }
  LimpiarCRLF(p->sDireccionIP);
  if(BindSocket(master_socket,(struct sockaddr*)&address,sizeof(address)) < 0)
  {
    perror("BindSocket");
    exit (EXIT_FAILURE);
  }
  if(ListenSocket(master_socket,10)<0)
  {
    perror("ListenSocket");
    exit (EXIT_FAILURE);
  }	
	addrlen = sizeof(address);
	Log(LOG_MENSAJE_EXTRA,p->fLog,"Comienza el control para saber cuando alguien necesita intercambiar datos con el server\n");
	//Comienza el control para saber cuando alguien necesita intercambiar datos con el server
	printf("Ya iniciamos el nuevo puerto de escucha sock: %d\n", master_socket);
	fflush(stdout);
	while(1) 
	{
		//Limpio el controlador de sockets
		FD_ZERO(&fdControl);
		FD_SET(master_socket, &fdControl);
		tv.tv_sec=3;
		tv.tv_usec=0;
		
		//agrego los clientes al controlador
		for ( i = 0 ; i < max_clients ; i++) 
		{
			if(client_socket[i]>0)
			{
				FD_SET(client_socket[i],&fdControl);
			}
		}
		
		if(select(maximoValor(client_socket)+1, &fdControl, NULL, NULL, &tv) == -1){
			perror("select");
			exit(1);
		}

		//Si pasa algo en el master_socket es una conexion nueva entrante
		if (FD_ISSET(master_socket, &fdControl)) 
		{
			Log(LOG_MENSAJE_EXTRA,p->fLog,"Si pasa algo en el master_socket es una conexion nueva entrante\n");
			new_socket=AcceptSocket(master_socket,(struct sockaddr*)&address,&addrlen);
			//Agrego el nuevo socket a la lista de sockets conectados
			for (i = 0; i < max_clients; i++) 
			{
				s = client_socket[i];
				if (s == 0)
				{
					client_socket[i] = new_socket;
					i = max_clients;
				}
			}
		}
		//Ahora controlo dentro del array de conexiones si el cliente o el server conectado quieren hablarme
		for (i = 0; i < max_clients; i++) 
		{
			s=client_socket[i];
			//printf("%d : %d \n",client_socket[i], s);	
			if(FD_ISSET(s,&fdControl)) 
			{
				//printf("Uno de los cliente intenta comunicarse\n");
				Log(LOG_MENSAJE_EXTRA,p->fLog,"Uno de los cliente intenta comunicarse\n");
				ManejarConexionConectada(s,p->fLog);
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
int InsertarListaConexion( int ifdsock, NODOClient *sClient, LISTACLIENT *Lista, int puerto, char * direccion)
{
  ListasClient sAux;

  sAux=(ListasClient)malloc(sizeof(NODOClient));
  if(sAux== NULL) 
     return -1;         /*Retorno -1 si hubo problemas con la memoria*/
	sAux = sClient;
	sAux->iSock=ifdsock;	
	sAux->sPuerto=puerto;
	memcpy(sAux->sDireccionIP,direccion,strlen(direccion));
  sAux->sig = *Lista;

  *Lista = sAux;
  return 1;               /*Retorno 1 si pude insertarlo bien*/
}

/*Recorre la lista de conexion y devuelve un contador de la cantidad de
elementos que contiene*/
int ContarListaConexion()
{
	int retval=0;

#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif
  LISTACLIENT sAux=lConexiones;
  while(sAux!= NULL && sAux->iSock!=0)
  {
		retval++;
    sAux=sAux->sig;
  }
#ifdef MUTEX
	/* Se desbloquea el mutex */
	pthread_mutex_unlock (&mutexBuffer);
#endif

	return retval;
}

/*Valida que un cliente este en la lista de conexiones*/
int EstaEnLista(char * sNombre)
{
	int retval=0;
#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif
	ListasClient sAux=lConexiones;
  while(sAux!=NULL && sAux->iSock!=0)
  {
     if(strcmp(sAux->sNombre,sNombre) == 0)
        retval=1;            /*Retorno 1 si lo encontro en la lista*/
     sAux=sAux->sig;
  }
#ifdef MUTEX
	/* Se desbloquea el mutex */
	pthread_mutex_unlock (&mutexBuffer);
#endif
	return retval;                  /*Retorno 0 si no lo encontro en la lista*/
}

/*Muestra la lista de conexiones*/
void MostrarLista(LISTACLIENT lista)
{
  ListasClient sAux=lista;

	while(sAux!=NULL && sAux->iSock!=0) 
	{
		printf("------------------------------");
		printf("|\n");
		printf("|\tnombre: %s |\n",sAux->sNombre);
		printf("|\tsock: %d |\n",sAux->iSock);
		printf("|\tdire IP: %s |\n",sAux->sDireccionIP);
		printf("|\tPuerto: %d |\n",sAux->sPuerto);
		if(sAux->iJugando==1)
			printf("|\tjugando: SI |\n");
		else
			printf("|\tjugando: NO |\n");
		printf("|\n");
		printf("------------------------------\n");
		fflush(stdout);
		sAux=sAux->sig;
	}
}

/*Esta funciona maneja la conexion con los clientes conectados al thread
que maneja la comunicacion con el cliente*/
void ManejarConexionConectada(int socket, FILE * fLog)
{
	char buf[BUFSIZE];
	char* command;
	int codigoComando,bytesRead;
  command=malloc(5);

  bzero(buf,BUFSIZE);
	Log(LOG_MENSAJE_EXTRA,fLog,"Esperando mensajes del cliente");
  bytesRead=ReadSocket(socket,buf,BUFSIZE,0);
  if(bytesRead >0)
  {
    LimpiarCRLF(buf);
    command=strtok(buf," ");
		printf("Comando: %s \n", command);
    codigoComando=EvaluarComando(command);
    switch(codigoComando)
    {
      case 1: /*LIST*/
						printf("Se esta enviando el listado a un cliente...\n");
						fflush(stdout);
						EnviarListaDeUsuarios(socket, fLog); /*Enviamos la lista de usuarios conectados*/
            break;
      case 2: /*GAME*/
						command=strtok(NULL," ");
						if(EstaEnLista(command)==1)
						{
							printf("Conectando al worker del usuario: %s", command);
							IniciarJuegoConWorker(command, fLog);
						}
						else
						{
							printf("El cliente solicitado se desconecto\n");
						}
            break;
			case 3: /*PLAY*/
						break;
      case 4: /*QUIT*/
            Quit(socket); /*Salir directamente*/
            break;
      default: /*Comando invalido*/
            ComandoInvalido(socket);
            break;
    }
  }
}

/*Primero calculo la cantidad de nodos en la lista, luego los envio uno a uno
al cliente mediante el socket enviado por parametro*/
void EnviarListaDeUsuarios(int socket, FILE* fLog)
{
	stHeader header;

	//Cargo la estructura
	strcpy(header.sMensaje,"LIST");
	header.iCantidad=ContarListaConexion();

	Log(LOG_MENSAJE_EXTRA,fLog,"Se envia al cliente la cantidad de nodos que tiene la lista\n");
	//Enviamos la cantidad de elementos a transmitir
  if(WriteSocket(socket,&header,sizeof(header),0)!=sizeof(header))
  {
    perror("ReadSocket");
    exit (EXIT_FAILURE);
	}
	printf("Envie la cantidad de nodos\n");

	//Comienzo a enviar la lista nodo a nodo
#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif
	ListasClient sAux=lConexiones;
	while(sAux!=NULL && sAux->iSock!=0)
	{
/*
		printf("Mostrando lo que estoy por enviar\n");
		MostrarClienteLista(sAux->sNombre, sAux->iSock, sAux->sDireccionIP, sAux->sPuerto, sAux->iJugando);
*/
		if(WriteSocket(socket,sAux,sizeof(sAux),0)!=sizeof(sAux))
		{
		  perror("ReadSocket");
		  exit (EXIT_FAILURE);
		}
		sAux=sAux->sig;
	}
#ifdef MUTEX
	/* Se desbloquea el mutex */
	pthread_mutex_unlock (&mutexBuffer);
#endif
	Log(LOG_MENSAJE_EXTRA,fLog,"Se envio al cliente la lista nodo a nodo\n");	
}

void IniciarJuegoConWorker(char * sNombre, FILE * fLog)
{
  int sockClient;
  struct sockaddr_in addrClient;
	sockClient=MakeSocket(AF_INET,SOCK_STREAM,0);

#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif
	ListasClient sAux=lConexiones;
  while(sAux)
  {
     if(strcmp(sAux->sNombre,sNombre) == 0)
        break;
  }
#ifdef MUTEX
	/* Se desbloquea el mutex */
	pthread_mutex_unlock (&mutexBuffer);
#endif
	
	//Me conecto con el puerto original de escucha del server
  bzero(&addrClient,sizeof(addrClient));
  addrClient.sin_family = AF_INET;
  addrClient.sin_port = htons(sAux->sPuerto);
	//deberia utilizar esto sAux->sDireccionIP
  if(inet_pton(AF_INET,"127.0.0.1",&addrClient.sin_addr) <0)
  {
    perror("inet_pton");
    exit (EXIT_FAILURE);
  }

  if (connect (sockClient, (struct sockaddr *)&addrClient, sizeof (addrClient)) == -1)
  {
    perror("Connect");
    Log(LOG_MENSAJE_EXTRA,fLog,sMsg_ErrorConexionDatos);
    exit (EXIT_FAILURE);
  }
	printf("Conectado con el worker en el socket: %d", sockClient);
}



