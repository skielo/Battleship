#include "log.h"
#include "configuracion.h"
#include "funciones_red.h"
#include "user.h"
#include "mensajes.h"
#include "client.h"
#include "funciones_print.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/time.h>

#define BUFSIZE 80
#define PUERTO 10001

#define LOG_MENSAJE_EXTRA -1
#define LOG_CONEXION_CLIENTE 1
#define LOG_COMANDO_CLIENTE 2
#define LOG_ERROR 3
#define LOG_CIERRE_SOCKET_LISTEN 4
#define LOG_CIERRE_SOCKET_ACCEPT 5
#define LOG_JUGADA_ENVIADA 6
#define LOG_JUGADA_RECIVIDA 7
#define LOG_CONEXION_SERVER 1

int main(int argc, char* argv[])
{
  char* sDireccion;
  char* sPuerto;
  int sockClient;
  struct sockaddr_in addrClient;
  FILE* fConfiguracion;
  FILE* fLog;
	char * sCodRespuesta;
	char sMsgRespuesta[BUFSIZE];
  fLog=ArchivoLog("./client.log");
  fConfiguracion=AbrirArchivo("./client.conf");
  sockClient=MakeSocket(AF_INET,SOCK_STREAM,0);
	sockClient=MakeSocket(AF_INET,SOCK_STREAM,0);
  sDireccion=malloc(80);
  sPuerto=malloc(6);
	int i,j;

	if(argc != 7)
	{
		printf("%s",sMsg_ParametroInvalido);
		exit (EXIT_SUCCESS);
	}

	printf("Mostrando el tablero de %s:\n", argv[1]);
	//Inicializo el cliente
	NODOClient this;

	strcpy(this.sNombre,argv[1]);
	for(i=0;i<10;i++) {
		for(j=0;j<10;j++) {
			this.iBoatTable[i][j] = 'a';
			this.iPlayTable[i][j] = 'a';
		}
	}
	for(i=1;i<argc;i++) 
	{
		this.iBoatTable[argv[i][0]-48][argv[i][1]-48] = 'x';
	}
  this.iSock=0;
  bzero(this.sDireccionIP,sizeof(this.sDireccionIP));
	this.sPuerto=0;
	this.iJugando=0;
	this.sig=NULL;

	//Muestro el mapa mio y del juego
	print_maps(this.iBoatTable,this.iPlayTable);

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

	//Me conecto con el puerto original de escucha del server
  bzero(&addrClient,sizeof(addrClient));
  addrClient.sin_family = AF_INET;
  addrClient.sin_port = htons(atoi(sPuerto));
  if(inet_pton(AF_INET,sDireccion,&addrClient.sin_addr) <0)
  {
    perror("inet_pton");
    exit (EXIT_FAILURE);
  }
  LimpiarCRLF(sDireccion);

  if (connect (sockClient, (struct sockaddr *)&addrClient, sizeof (addrClient)) == -1)
  {
    perror("Connect");
    Log(LOG_MENSAJE_EXTRA,fLog,sMsg_ErrorConexionDatos);
    exit (EXIT_FAILURE);
  }

  printf("Cliente Battleship V 1.1\n-----------");
  printf("\nConectando con el servidor en %s:%s\n",sDireccion,sPuerto);

  Log(LOG_CONEXION_SERVER,fLog,"");
  printf("\nConexion con el Servidor aceptada\n-----------------\n");

  if(ReadSocket(sockClient,sMsgRespuesta,6,0)<0)
  {
  	perror("ReadSocket");
		Log(LOG_MENSAJE_EXTRA,fLog,"Error logeandose al servidor\n");
		exit (EXIT_FAILURE);
  }
  sCodRespuesta=strtok(sMsgRespuesta," ");
  if(strcmp(sCodRespuesta,"OK")!=0)
  {
		Log(LOG_MENSAJE_EXTRA,fLog,"Login: Error en la respuesta de USER\n");
		exit (EXIT_FAILURE);
  }

	printf("%s",sMsg_Bienvenida);
	fflush(stdout);
	ControlDeConexion(sockClient,sDireccion,fLog,this);

  fclose(fLog);
  return EXIT_SUCCESS;
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

void ComandoInvalido(void)
{
	printf("%s",sMsg_ComandoInvalido);
	fflush(stdout);
}

int EvaluarComando(char* sComando)
{
	if(sComando == NULL)
		sComando = " ";
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


void ControlDeConexion(int Descriptor,const char* sDireccionIP,FILE* fLog, NODOClient cliente)
{
	char* command;
	char buffer[20];
	int codigoComando;
	fd_set master;
	struct timeval tv;

	cliente.iSock=Descriptor;
	//Enviamos al server la informacion de nuestro cliente
  if(WriteSocket(Descriptor,&cliente,sizeof(cliente),0)!=sizeof(cliente))
  {
    perror("ReadSocket");
    exit (EXIT_FAILURE);
	}

	fflush(stdin);
	printf("Jugador>");	
	fflush(stdout);

  while(1)
  {	
		FD_ZERO(&master);
		FD_SET(Descriptor, &master);
		FD_SET(0, &master);
		tv.tv_sec=3;
		tv.tv_usec=0;
		
		if(select(Descriptor+1, &master, NULL, NULL, &tv) == -1){
			perror("select");
			exit(1);
		}
		if(FD_ISSET(0,&master)) //Se presiono el teclado (ENTER)
		{
			Log(LOG_MENSAJE_EXTRA,fLog,"Esperando comando del jugador\n");
			//El usuario esta tratando de scribir
			gets(buffer);
			if(buffer >0)
			{
				command=strtok(buffer," ");
				codigoComando=EvaluarComando(command);
				switch(codigoComando)
				{
				  case 1: /*LIST*/
								printf("Queres listar los usuarios\n");
								fflush(stdout);
								/*Se procede a pedir al servidor que nos envie la lista de los usuarios conectados*/	
								ListarUsuariosDelServidor(Descriptor,fLog);							
				        break;
				  case 2: /*GAME*/
								command=strtok(NULL," ");
								printf("Queres iniciar un juego con: %s\n",command);
								/*Indicamos al servidor el nombre del jugador con el que queremos jugar*/
								IniciarJuegoCon(Descriptor, command,fLog);
				        break;
				  case 3: /*PLAY*/
								printf("Queres jugar con X\n");
				        command=strtok(NULL," ");
				        break;
					case 4: /*QUIT*/
								printf("%s",sMsg_Quit);
								CloseSocket(Descriptor);
								exit (EXIT_SUCCESS);
				  default: /*Comando invalido*/
				        ComandoInvalido();
				        break;
				}
			}
			printf("Jugador>");	
		}
		if(FD_ISSET(Descriptor, &master)){
			if(ReadSocket(Descriptor,command,50,0)<0)
			{
				perror("ReadSocket");
				Log(LOG_MENSAJE_EXTRA,fLog,"Error logeandose al servidor\n");
				exit (EXIT_FAILURE);
			}
		}
  }
}

/*Esta funcion se encarga de enviar el pedido al servidor de la lista de usuarios
conectados, tambien la imprime*/
void ListarUsuariosDelServidor(int iSocket, FILE * fLog)
{
	int cantElement, i=1;
	ListasClient nodoRecibido;
	stHeader header;
	
 	//Genero la estructura con el mensansaje
	strcpy(header.sMensaje,"LIST");
	//Enviamos al server el pedido del listado
  if(WriteSocket(iSocket,&header,sizeof(header),0)!=sizeof(header))
  {
    perror("ReadSocket");
    exit (EXIT_FAILURE);
	}
	printf("Espero la respuesta de la lista\n");
  if(ReadSocket(iSocket,&header,sizeof(header),0)<0)
  {
  	perror("ReadSocket");
		Log(LOG_MENSAJE_EXTRA,fLog,"Error logeandose al servidor\n");
		exit (EXIT_FAILURE);
  }
	cantElement=header.iCantidad;
	printf("La cantidad a recibir es: %d\n",cantElement);
	while(i<=cantElement)
	{
		nodoRecibido=(ListasClient)malloc(sizeof(NODOClient));
		if(ReadSocket(iSocket,nodoRecibido,sizeof(nodoRecibido),0)<0)
		{
			perror("ReadSocket");
			Log(LOG_MENSAJE_EXTRA,fLog,"Error logeandose al servidor\n");
			exit (EXIT_FAILURE);
		}
		MostrarClienteLista(nodoRecibido->sNombre, nodoRecibido->iSock, nodoRecibido->sDireccionIP, nodoRecibido->sPuerto, nodoRecibido->iJugando);
		i++;
	}
}

void IniciarJuegoCon(int iSocket, char * sNombre,FILE * fLog)
{
	stHeader header;
	
 	//Genero la estructura con el mensansaje
	strcpy(header.sMensaje,"GAME ");
	strcat(header.sMensaje,sNombre);
	//Enviamos al server el pedido del listado
  if(WriteSocket(iSocket,&header,sizeof(header),0)!=sizeof(header))
  {
    perror("ReadSocket");
    exit (EXIT_FAILURE);
	}
}


