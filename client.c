#include "log.h"
#include "configuracion.h"
#include "funciones_red.h"
#include "user.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "mensajes.h"
#include "client.h"
#include <ctype.h>

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

int main(int argc, char** argv)
{
  char* sDireccion;
  char* sPuerto;
  int Descriptor, sockClient;
  struct sockaddr_in addrListen, addrClient;
  FILE* fConfiguracion;
  FILE* fLog;
	char * sCodRespuesta;
	char sMsgRespuesta[BUFSIZE];
  fLog=ArchivoLog("./client.log");
  fConfiguracion=AbrirArchivo("./client.conf");
  Descriptor=MakeSocket(AF_INET,SOCK_STREAM,0);
	sockClient=MakeSocket(AF_INET,SOCK_STREAM,0);
  sDireccion=malloc(80);
  sPuerto=malloc(6);

	if(argc != 3)
	{
		printf("%s",sMsg_ParametroInvalido);
		exit (EXIT_SUCCESS);
	}

	stClient this = MakeClient(argv[1], argv[2],  fConfiguracion);

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

  if (connect (Descriptor, (struct sockaddr *)&addrClient, sizeof (addrClient)) == -1)
  {
    perror("Connect");
    Log(LOG_MENSAJE_EXTRA,fLog,sMsg_ErrorConexionDatos);
    exit (EXIT_FAILURE);
  }

  printf("Cliente Battleship V 0.1\n-----------");
  printf("\nConectando con el servidor en %s:%s\n",sDireccion,sPuerto);

  Log(LOG_CONEXION_SERVER,fLog,"");
  printf("\nConexion con el Servidor aceptada\n-----------------\n");
  if(ReadSocket(Descriptor,sMsgRespuesta,50,0)<0)
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

	ControlDeConexion(sockClient,sDireccion,sCodRespuesta,fConfiguracion);

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
	fflush(stdout);
	printf("%s",sMsg_ComandoInvalido);
}

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


void ControlDeConexion(int Descriptor,const char* sDireccionIP,const char* sPuerto,FILE* fConfiguracion)
{
	char* command;
	char buffer[20];
	int codigoComando;

  fclose(fConfiguracion);
  fConfiguracion=AbrirArchivo("./client.conf");
	
  while(1)
  {
		fflush(stdout);
		fflush(stdin);
		printf("Jugador>");
		gets(buffer);
    if(buffer >0)
    {
      command=strtok(buffer," ");
      codigoComando=EvaluarComando(command);
      switch(codigoComando)
      {
        case 1: /*LIST*/
							printf("Queres listar los usuarios\n");
              command=strtok(NULL," ");
              break;
        case 2: /*GAME*/
							printf("Queres mostrar el juego\n");
							command=strtok(NULL," ");
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
  }
}
