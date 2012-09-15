#include "log.h"
#include "configuracion.h"
#include "funciones_red.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "mensajes.h"
#include "ftpserver.h"
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


int main(int argc, char** argv)
{
  char* sDireccion;
  char* sPuerto;
  int sockListen,sockClient;
  struct sockaddr_in addrListen, addrClient;
  socklen_t clilen;
  pid_t childpid;
  FILE* fConfiguracion;
  FILE* fLog;
	pthread_t idChild; 
  fLog=ArchivoLog("./server.log");
  fConfiguracion=AbrirArchivo("./server.conf");
  sockListen=MakeSocket(AF_INET,SOCK_STREAM,0);
  sDireccion=malloc(80);
  sPuerto=malloc(6);
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
  bzero(&addrListen,sizeof(addrListen));
  addrListen.sin_family = AF_INET;
  addrListen.sin_port = htons(atoi(sPuerto));
  if(inet_pton(AF_INET,sDireccion,&addrListen.sin_addr) <0)
  {
    perror("inet_pton");
    exit (EXIT_FAILURE);
  }
  GenerarPuerto(atoi(sPuerto));
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
  clilen=sizeof(addrClient);
  printf("Servidor de juevo V 0.1\n-----------");
  printf("\nEsperando conexiones de clientes en %s:%s\n",sDireccion,sPuerto);
  while(1)
  {
    sockClient=AcceptSocket(sockListen,(struct sockaddr*)&addrClient,&clilen);
    Log(LOG_CONEXION_CLIENTE,fLog,"");
    printf("\nConexion entrante aceptada\n-----------------\n");
    WriteSocket(sockClient,"220 \r\n",strlen("220 \r\n"),0);
    if((pthread_create (&idChild, NULL, ConexionControl(sockClient,sDireccion,sPuerto,fConfiguracion), NULL);) == 0)
    {
      fclose(fLog);
      CloseSocket(sockListen);
      break;
    }
    else
    {
			perror("FORK");
      fclose(fConfiguracion);
    }

  }
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

/*Esta funcion se encarga de manejar la conexion con el cliente, esperando
comandos desde el mismo y evaluandolos*/
void * ConexionControl(int sockClient,const char* sDireccionIP,const char* sPuerto,FILE* fConfiguracion)
{
int bytesRead;
char buf[BUFSIZE];
char auxbuf[BUFSIZE];
char* command;
int codigoComando;
int iResultado;
int sockDTP;

  fclose(fConfiguracion);
  fConfiguracion=AbrirArchivo("./ftpserver.conf");
  command=malloc(5);
  while(1)
  {
    bzero(buf,BUFSIZE);
    bytesRead=ReadSocket(sockClient,buf,BUFSIZE,0);
    if(bytesRead >0)
    {
      LimpiarCRLF(buf);
      command=strtok(buf," ");
      printf("Comando: %s \n",buf);
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
              Quit(sockClient); /*Salir directamente*/
              return;
        default: /*Comando invalido*/
              ComandoInvalido(sockClient);
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
