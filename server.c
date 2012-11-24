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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSIZE 1025
#define PUERTO 10001

#define LOG_MENSAJE_EXTRA -1
#define LOG_CONEXION_CLIENTE 1
#define LOG_COMANDO_CLIENTE 2
#define LOG_ERROR 3
#define LOG_CIERRE_SOCKET_LISTEN 4
#define LOG_CIERRE_SOCKET_ACCEPT 5
#define LOG_JUGADA_ENVIADA 6
#define LOG_JUGADA_RECIBIDA 7
#define MAXLENGHT 25
#define MEM_SZ  4096

#ifdef MUTEX
/* mutex para sincronizar el acceso a buffer */
pthread_mutex_t mutexBuffer;
#endif
//Variable global con la lista de conexiones activdas
LISTACLIENT lConexiones=NULL;
struct shared_use_st * shared_stuff; 

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
/*Parte del refactor*/
	NODOClient * cliente;
	fd_set fdControl;
	struct timeval tv;
	int newPort=0,bytesRead, newSock=0;
	char newDireccion[MAXLENGHT];
  ListasClient sAux;

/*Inicio Memoria compartida*/
	void * shared_memory = (void *) 0;
	
	int shmid;

	shmid = shmget((key_t)1234, MEM_SZ, IPC_CREAT | 0666);
	if (shmid == -1) {
	 fprintf (stderr, "shmget failed \n");
	 exit (EXIT_FAILURE);
	}

	shared_memory = shmat(shmid, (void *) 0, 0);
	if (shared_memory == (void *) -1) {
	 fprintf (stderr, "shmat failed \n");
	 exit (EXIT_FAILURE);
	}

	shared_stuff = (struct shared_use_st *) shared_memory; 

	strcpy(shared_stuff->sUser," ");
	strcpy(shared_stuff->mensaje," ");
/*Fin memoria compartida*/

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

  //bzero(&addrListen,sizeof(addrListen));
  addrListen.sin_family = AF_INET;
	addrListen.sin_addr.s_addr = INADDR_ANY;
  addrListen.sin_port = htons(atoi(sPuerto));
/*
  if(inet_pton(AF_INET,sDireccion,&addrListen.sin_addr) <0)
  {
    perror("inet_pton");
    exit (EXIT_FAILURE);
  }
*/
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
	fflush(stdout);
	printf("Esperando nuevas conexiones\n");
  while(1)
  {
		/*
				printf("-----------------Los conectados hasta el momento-----------------\n");
				MostrarLista(lConexiones);
		*/

		/*ACA EMPIEZA EL REFACTORING QUE ESTOY HACIENDO*/
		//Limpio el controlador de sockets
		FD_ZERO(&fdControl);
		FD_SET(sockListen,&fdControl);
	#ifdef MUTEX
		/* Se espera y se bloquea el mutex */
		pthread_mutex_lock (&mutexBuffer);
	#endif
		sAux=lConexiones;
		while(lConexiones!=NULL && sAux->iSock!=0)
		{
			FD_SET(sAux->iSock,&fdControl);
		  sAux=sAux->sig;
		}
	#ifdef MUTEX
		/* Se desbloquea el mutex */
		pthread_mutex_unlock (&mutexBuffer);
	#endif
		tv.tv_sec=3;
		tv.tv_usec=0;
		//printf("el valor maximo: %d\n",maximoValor()+3);
		int activity=0;
		int maxVal = maximoValor();
		if(maxVal<sockClient)
		{
			maxVal=sockClient;
		}
		activity=select(maxVal+3, &fdControl, NULL, NULL, &tv);
		if(activity == -1){
			perror("select");
			exit(1);
		}
		//printf("activity: %d\n",activity);
		//printf("el socket list:%d\n",sockListen);
		if (FD_ISSET(sockListen, &fdControl)) 
		{
		  sockClient=AcceptSocket(sockListen,(struct sockaddr*)&addrClient,&clilen);
		  Log(LOG_CONEXION_CLIENTE,fLog,"");
		  printf("\nConexion entrante aceptada\n-----------------\n");

			//Enviamos la confirmacion al cliente y le decimos donde debe conectarse
		  if(WriteSocket(sockClient,sCommonAnswer,strlen(sCommonAnswer),0)!=strlen(sCommonAnswer))
			{
				perror("WriteSocket");
				exit (EXIT_FAILURE);
			}

			Log(LOG_MENSAJE_EXTRA,fLog,"Se envio al cliente la respuesta comun");

			cliente=(NODOClient *)malloc(sizeof(NODOClient));
			//Recibo los datos que me va a mandar el cliente sobre el mismo
			bytesRead=ReadSocket(sockClient,cliente,sizeof(NODOClient),0);

			if(bytesRead <0)
			{
				perror("ReadSocket");
				exit (EXIT_FAILURE);
			}
			Log(LOG_MENSAJE_EXTRA,fLog,"Recibo los datos que me va a mandar el cliente sobre el mismo\n");
			printf("Se conecto: %s en el socket: %d\n",cliente->sNombre, sockClient);
			//Genero el nuevo puerto donde deben conectarse
			newPort = GenerarPuerto(-1);
			//printf("conectando en %s:%d\n", p->sDireccionIP,newPort);
			fflush(stdout);
		#ifdef MUTEX
			/* Se espera y se bloquea el mutex */
			pthread_mutex_lock (&mutexBuffer);
		#endif

			newSock=sockClient;
			memcpy(newDireccion,sDireccion,strlen(sDireccion));
			//Insert el nuevo cliente en la lista de conexiones compartida
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
			Log(LOG_MENSAJE_EXTRA,fLog,"Insert el nuevo cliente en la lista de conexiones compartida\n");
			Log(LOG_MENSAJE_EXTRA,fLog,"Inicio la conexion de control para comunciarme con el cliente y con otros server que quieran jugar con este cliente\n");
			//fflush(stdout);
		}
		//Reviso la memoria compartida para ver si tengo que escribirle a mi cliente
		#ifdef MUTEX
			/* Se espera y se bloquea el mutex */
			pthread_mutex_lock (&mutexBuffer);
		#endif
			sAux=lConexiones;
			while(sAux)
			{
				if(strcmp(sAux->sNombre,shared_stuff->sUser) == 0)
					break;
				sAux=sAux->sig;
			}
		#ifdef MUTEX
			/* Se desbloquea el mutex */
			pthread_mutex_unlock (&mutexBuffer);
		#endif
			//printf("El control lo tiene: %s\n",shared_stuff->sUser);
			if(sAux!=NULL)
			{
				if(strcmp(shared_stuff->mensaje,"GAME")==0 && strcmp(sAux->sNombre,shared_stuff->sUser) == 0)
				{
					char temp[10];
					printf("El control lo tiene: %s\n",shared_stuff->sUser);
					EnviarInicioJuegoCliente(sAux->iSock,fLog);
					strcpy(temp,shared_stuff->sUser);
					strcpy(shared_stuff->sUser,shared_stuff->sOponente);
					strcpy(shared_stuff->sOponente,temp);
					strcpy(shared_stuff->mensaje,"PLAY");
					MarcarClienteJugando(sAux->iSock);
					printf("Se sedio el control a: %s\n",shared_stuff->sUser);
				}
				if(strcmp(shared_stuff->mensaje,"PLAY")==0 && strcmp(sAux->sNombre,shared_stuff->sUser) == 0)
				{
					//Envio el PLAY a mi cliente
					printf("Envio el play a : %s, para que juegue contra: %s\n",sAux->sNombre,shared_stuff->sOponente);
					EnviarPlayCliente(sAux->iSock,shared_stuff->sOponente, fLog);
				}	
				if(strcmp(shared_stuff->mensaje,"ANS")==0 && strcmp(sAux->sNombre,shared_stuff->sUser) == 0)
				{
					//Envio el ANS a mi cliente
					printf("Envio el ANS a : %s\n",sAux->sNombre);
					EnviarANSCliente(sAux->iSock, fLog);
					strcpy(shared_stuff->sUser,sAux->sNombre);
					strcpy(shared_stuff->mensaje,"PLAY");
				}	
			}		
		//Ahora controlo dentro del array de conexiones si el cliente o el server conectado quieren hablarme
		#ifdef MUTEX
			/* Se espera y se bloquea el mutex */
			pthread_mutex_lock (&mutexBuffer);
		#endif
			sAux=lConexiones;
			while(lConexiones!=NULL && sAux->iSock!=0)
			{
				//printf("controlo el socket %d\n", sAux->iSock);
				if(FD_ISSET(sAux->iSock,&fdControl)) 
				{
					//printf("El socket: %d quiere comunicarse\n",sAux->iSock);
					//printf("Uno de los cliente intenta comunicarse\n");
					//Log(LOG_MENSAJE_EXTRA,p->fLog,"Uno de los cliente intenta comunicarse\n");
					//ManejarConexionConectada(sAux->iSock,fLog);
					param=ArmarParametros(sAux->iSock,sDireccion,sPuerto,fLog);
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
				sAux=sAux->sig;
			}
		#ifdef MUTEX
			/* Se desbloquea el mutex */
			pthread_mutex_unlock (&mutexBuffer);
		#endif

/*EL REFACTOR VA HASTA ACA*/
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

int maximoValor()
{
	int maximo=0;
  ListasClient sAux;
	#ifdef MUTEX
		/* Se espera y se bloquea el mutex */
		pthread_mutex_lock (&mutexBuffer);
	#endif
		sAux=lConexiones;
		while(lConexiones!=NULL && sAux->iSock!=0)
		{
			maximo++;
		  sAux=sAux->sig;
		}
	#ifdef MUTEX
		/* Se desbloquea el mutex */
		pthread_mutex_unlock (&mutexBuffer);
	#endif
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

	//Luego de iniciado el thread manejo la conexion con el cliente
	ManejarConexionConectada(p->sockClient,p->fLog);
	return 0;
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

  if(strcmp(sComando,"SERV")==0)
    return 5;

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
	printf("Estoy buscando a : %s\n", sNombre);
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

/*Cambia el estado de un cliente a JUGANDO*/
void MarcarClienteJugando(int iSock)
{
#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif
	ListasClient sAux=lConexiones;
  while(lConexiones!=NULL && sAux->iSock!=0)
  {
		if(sAux->iSock==iSock)
		{
			sAux->iJugando=1;
			printf("Se puso al cliente %s a jugar\n",sAux->sNombre);
			fflush(stdout);
			//cambio=1;
			break;
		}
    sAux=sAux->sig;
  }
#ifdef MUTEX
	/* Se desbloquea el mutex */
	pthread_mutex_unlock (&mutexBuffer);
#endif
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
		printf("Mapa:\n");
		print_maps(sAux->iBoatTable,sAux->iPlayTable);
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
  command=malloc(MAXLENGHT);
	stHeader header;

  bzero(buf,BUFSIZE);
	Log(LOG_MENSAJE_EXTRA,fLog,"Esperando mensajes del cliente\n");
  bytesRead=ReadSocket(socket,&header,sizeof(header),0);//ReadSocket(socket,buf,BUFSIZE,0);

  if(bytesRead >0)
  {
    LimpiarCRLF(header.sMensaje);
    command=strtok(header.sMensaje," ");
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
						printf("Buscando a : %s\n", command);
						if(EstaEnLista(command)==1)
						{
							printf("Conectando al worker del usuario: %s\n", command);
							/*Busco el nombre de mi usuario*/
							#ifdef MUTEX
								/* Se espera y se bloquea el mutex */
								pthread_mutex_lock (&mutexBuffer);
							#endif
							ListasClient sAux=lConexiones;
							while(lConexiones!=NULL && sAux->iSock!=0)
							{
								if(sAux->iSock==socket)
									break;
								sAux=sAux->sig;
							}
							#ifdef MUTEX
								/* Se desbloquea el mutex */
								pthread_mutex_unlock (&mutexBuffer);
							#endif
							IniciarJuegoConWorker(command,sAux->sNombre, fLog);
							MarcarClienteJugando(socket);
						}
						else
						{
							printf("El cliente solicitado se desconecto\n");
						}
            break;
			case 3: /*PLAY*/
						//Proceso la jugada
						command=strtok(NULL," ");
						printf("Recibi una jugada de mi cliente, jugando contra: %s\n",command);
						RecibirJugadaCliente(socket,command, fLog);
						break;
      case 4: /*QUIT*/
            Quit(socket); /*Salir directamente*/
            break;
      case 5: /*SERV*/
            /*Recibo un mensaje desde otro worker*/
            break;
      default: /*Comando invalido*/
            ComandoInvalido(socket);
            break;
    }
  }
	else
  {
  	perror("ReadSocket");
		fflush(stdout);
		Log(LOG_MENSAJE_EXTRA,fLog,"Error recibiendo mensajes del cliente\n");
		exit (EXIT_FAILURE);
  }
}

/*Recibe y procesa la jugada del cliente*/
void RecibirJugadaCliente(int iSock,char * sOponente, FILE* fLog)
{
	NODOClient * cliente;
	int i=0, j=0, hundido=0,bytesRead;
	ListasClient sAux;
	stHeader header;

	/*Busco el nombre de mi usuario*/
#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif
	sAux=lConexiones;
	while(sAux!=NULL && sAux->iSock!=0)
	{
		if(strcmp(sAux->sNombre,sOponente)==0)
			break;
		sAux=sAux->sig;
	}
#ifdef MUTEX
	/* Se desbloquea el mutex */
	pthread_mutex_unlock (&mutexBuffer);
#endif
	//Recibo los datos que me va a mandar el cliente sobre el mismo
	cliente=(NODOClient *)malloc(sizeof(NODOClient));
	bytesRead=ReadSocket(iSock,cliente,sizeof(NODOClient),0);
	if(bytesRead <0)
	{
		perror("ReadSocket");
		exit (EXIT_FAILURE);
	}
	for(i=0;i<10;i++) {
		for(j=0;j<10;j++) {
			if(cliente->iPlayTable[i][j] == 'x')
			{
				if(sAux->iBoatTable[i][j] == 'x')
				{
					cliente->iPlayTable[i][j] = 'h';
					hundido=1;
				}
				else
					cliente->iPlayTable[i][j] = 'f';
			}
		}
	}
	//Actualizo mi cliente en la lista
	/*Busco el nombre de mi usuario*/
#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif
	sAux=lConexiones;
	while(sAux!=NULL && sAux->iSock!=0)
	{
		if(strcmp(sAux->sNombre,cliente->sNombre)==0)
			break;
		sAux=sAux->sig;
	}
#ifdef MUTEX
	/* Se desbloquea el mutex */
	pthread_mutex_unlock (&mutexBuffer);
#endif
	for(i=0;i<10;i++) {
		for(j=0;j<10;j++) {
			if(cliente->iPlayTable[i][j] == 'h')
				sAux->iPlayTable[i][j] = 'h';
			if(cliente->iPlayTable[i][j] == 'f')
				sAux->iPlayTable[i][j] = 'f';
		}
	}
	
	//Envio el resultado de la jugada a mi cliente y copio la informacion al worker del oponente
	if(hundido==1)
	{
		//Cargo la estructura
		strcpy(header.sMensaje,"hundido");
		Log(LOG_MENSAJE_EXTRA,fLog,"el resultado de la jugada fue hundido\n");
	}
	else
	{
		//Cargo la estructura
		strcpy(header.sMensaje,"fallo");
		Log(LOG_MENSAJE_EXTRA,fLog,"el resultado de la jugada fue fallido\n");
	}
	if(WriteSocket(iSock,&header,sizeof(header),0)!=sizeof(header))
	{
	  perror("WriteSocket");
	  exit (EXIT_FAILURE);
	}
	printf("El control lo tenia: %s, Entregando el control a:%s\n", cliente->sNombre, sOponente);
	strcpy(shared_stuff->sUser,sOponente);
	strcpy(shared_stuff->sOponente,cliente->sNombre);
	strcpy(shared_stuff->mensaje,"ANS");
	shared_stuff->hundido = hundido;
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
    perror("WriteSocket");
    exit (EXIT_FAILURE);
	}
	printf("Envie la cantidad de nodos: %d\n", header.iCantidad);

	//Comienzo a enviar la lista nodo a nodo
#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif
	ListasClient sAux=lConexiones;
	printf("Mostrando lo que estoy por enviar\n");
	while(sAux!=NULL && sAux->iSock!=0)
	{
		MostrarClienteLista(sAux->sNombre, sAux->iSock, sAux->sDireccionIP, sAux->sPuerto, sAux->iJugando);

		if(WriteSocket(socket,sAux,sizeof(NODOClient),0)!=sizeof(NODOClient))
		{
		  perror("WriteSocket");
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

/*Esta funcion es la encargada de realizar la conexion con el worker que atiene al cliente, en
esta conexion es la que vamos a utilizar para realizar la comunicacion cliente-cliente*/
void IniciarJuegoConWorker(char * sNombre, char * sOponente, FILE * fLog)
{
#ifdef MUTEX
	/* Se espera y se bloquea el mutex */
	pthread_mutex_lock (&mutexBuffer);
#endif
	ListasClient sAux=lConexiones;
  while(sAux)
  {
		if(strcmp(sAux->sNombre,sNombre) == 0)
			break;
		sAux=sAux->sig;
  }
#ifdef MUTEX
	/* Se desbloquea el mutex */
	pthread_mutex_unlock (&mutexBuffer);
#endif
	printf("Conectando con el cliente:\n");
	fflush(stdout);
	MostrarClienteLista(sAux->sNombre, sAux->iSock, sAux->sDireccionIP, sAux->sPuerto, sAux->iJugando);
	strcpy(shared_stuff->sUser,sAux->sNombre);
	strcpy(shared_stuff->mensaje,"GAME");
	strcpy(shared_stuff->sOponente,sOponente);
/*
	shared_stuff->sDireccion[25];
	shared_stuff->iPuerto;
*/
	printf("Se envio la informacion al worker del usuario: %s\n", sAux->sNombre);
	fflush(stdout);
}

/*Primero calculo la cantidad de nodos en la lista, luego los envio uno a uno
al cliente mediante el socket enviado por parametro*/
void EnviarInicioJuegoCliente(int socket, FILE* fLog)
{
	stHeader header;

	//Cargo la estructura
	strcpy(header.sMensaje,"GAME");

	Log(LOG_MENSAJE_EXTRA,fLog,"Se inicio un nuevo juego\n");
	//Enviamos la cantidad de elementos a transmitir
  if(WriteSocket(socket,&header,sizeof(header),0)!=sizeof(header))
  {
    perror("WriteSocket");
    exit (EXIT_FAILURE);
	}
	printf("Envie la confirmacion del juego a mi cliente\n");
	fflush(stdout);
}

/*Primero calculo la cantidad de nodos en la lista, luego los envio uno a uno
al cliente mediante el socket enviado por parametro*/
void EnviarPlayCliente(int socket, char * sOponente, FILE* fLog)
{
	stHeader header;

	//Cargo la estructura
	strcpy(header.sMensaje,"PLAY");
	strcpy(header.sNombre,sOponente);
	header.iCantidad=shared_stuff->hundido;
	Log(LOG_MENSAJE_EXTRA,fLog,"Se envia el PLAY al cliente\n");
	//Enviamos el play junto al nombre del oponente
  if(WriteSocket(socket,&header,sizeof(header),0)!=sizeof(header))
  {
    perror("WriteSocket");
    exit (EXIT_FAILURE);
	}
	strcpy(shared_stuff->mensaje," ");
	strcpy(shared_stuff->sUser," ");
	printf("Esperando la jugada de mi cliente\n");
	fflush(stdout);
}

/*Envia al cliente el resultado de la jugada*/
void EnviarANSCliente(int socket, FILE* fLog)
{
	stHeader header;

	//Cargo la estructura
	strcpy(header.sMensaje,"ANSW");
	header.iCantidad=shared_stuff->hundido;
	Log(LOG_MENSAJE_EXTRA,fLog,"Se envia el ANS al cliente\n");
	//Enviamos el ANS
  if(WriteSocket(socket,&header,sizeof(header),0)!=sizeof(header))
  {
    perror("WriteSocket");
    exit (EXIT_FAILURE);
	}
	printf("Envie el resultado de la jugada a mi cliente\n");
	fflush(stdout);
}


