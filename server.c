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
	char sNum[5];
	char sCommonAnswer[10] = "OK \r\n";
  int sockListen,sockClient,sockClientNew, iThread, iPort;
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
	GenerarPuerto(atoi(sPuerto));
  clilen=sizeof(addrClient);
  printf("Servidor de juego V 0.1\n-----------");
  printf("\nEsperando conexiones de clientes en %s:%s\n",sDireccion,sPuerto);
	printf("--------------------------------------------------------------\n");
  while(1)
  {
		fflush(stdout);
		printf("Esperando nuevas conexiones\n");
		printf("Esperando en el socket: %d",sockListen);
    sockClient=AcceptSocket(sockListen,(struct sockaddr*)&addrClient,&clilen);
    Log(LOG_CONEXION_CLIENTE,fLog,"");
    printf("\nConexion entrante aceptada\n-----------------\n");
/*
		//Generamos el nuevo puesto de escucha
	  iPort=GenerarPuerto(-1);
		itoa(iPort, sNum);
		strcat(sCommonAnswer,sNum);
		strcat(sCommonAnswer," \r\n");
		//printf("%s\n",sCommonAnswer);
*/
		//Enviamos la confirmacion al cliente y le decimos donde debe conectarse
    WriteSocket(sockClient,sCommonAnswer,strlen(sCommonAnswer),0);
/*
		//Ingresar la funcion que hace el llamado
		sockClientNew=CrearConexionControl(iPort, sDireccion, fLog);
		printf("La nueva conexion es el socket: %d\n",sockClientNew);
		CloseSocket(sockClient);

		itoa(iPort, sNum);
		printf("socket: %d, ip: %s, puerto: %s\n",sockClientNew, sDireccion, sNum);
*/
		param=ArmarParametros(sockClient,sDireccion,sPuerto,fConfiguracion);
		iThread = pthread_create(&idChild, NULL, ConexionControl, &param);
		printf("nuevo thread creado\n");
    if(iThread != 0)
    {
			perror("THREAD");
      fclose(fConfiguracion);
      fclose(fLog);
      CloseSocket(sockListen);
      break;
    }
		LimpiarAnswer(sCommonAnswer);
		//printf("despues de limpiar: %s\n",sCommonAnswer);
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

void LimpiarAnswer(char * answer)
{
	int x;
	for(x=3;x<strlen(answer);x++)
		answer[x]=NULL;
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

void itoa(int n, char s[])
{
     int i, sign;

     if ((sign = n) < 0)
         n = -n;         
     i = 0;
     do {       
         s[i++] = n % 10 + '0';
     } while ((n /= 10) > 0); 
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}

void reverse(char s[])
{
     int i, j;
     char c;

     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
}


/*Arma los parametros para iniciar el nuevo thread*/
stParam ArmarParametros(int sockClient,char* sDireccionIP,char* sPuerto,FILE* fConfiguracion)
{
	//printf("socket: %d, ip: %s, puerto: %s\n",sockClient, sDireccionIP, sPuerto);
	stParam retval;
	retval.sockClient=sockClient;
  memset(retval.sPuerto,(unsigned char)sPuerto, 1);
	retval.sDireccionIP = malloc(sizeof(sDireccionIP));
  strcpy(retval.sDireccionIP, sDireccionIP);
  retval.fConfiguracion=fConfiguracion;
	return retval;
}

int CrearConexionControl(int iPort, char * sDireccion, FILE* fLog)
{
	int retval,sockListenNew;
	struct sockaddr_in addrListenNew, addrClientNew;
	socklen_t client;

	//Generamos el nuevo socket de escucha
	sockListenNew=MakeSocket(AF_INET,SOCK_STREAM,0);
	bzero(&addrListenNew,sizeof(addrListenNew));
	addrListenNew.sin_family = AF_INET;
	addrListenNew.sin_port = htons(iPort);

	if(inet_pton(AF_INET,sDireccion,&addrListenNew.sin_addr) <0)
	{
	  perror("inet_pton");
	  exit (EXIT_FAILURE);
	}
	LimpiarCRLF(sDireccion);

	if(BindSocket(sockListenNew,(struct sockaddr*)&addrListenNew,sizeof(addrListenNew)) < 0)
	{
	  perror("BindSocket");
	  Log(LOG_MENSAJE_EXTRA,fLog,"Error Estableciendo la Conexion de Escucha\n");
	  exit (EXIT_FAILURE);
	}

	if(ListenSocket(sockListenNew,10)<0)
	{
	  perror("ListenSocket");
	  Log(LOG_MENSAJE_EXTRA,fLog,"Error Escuchando En La Conexion\n");
	  exit (EXIT_FAILURE);
	}
	client=sizeof(addrClientNew);
	return AcceptSocket(sockListenNew,(struct sockaddr*)&addrClientNew,&client);
}

// BindSocket: Address already in use
// socket servidor seterarle un flag SO_REUSEADDR setsockopt


/*Esta funcion se encarga de manejar la conexion con el cliente, esperando
comandos desde el mismo y evaluandolos*/
void * ConexionControl(void * param)
{
	stParam * p = (stParam *)param;
	int bytesRead;
	char buf[BUFSIZE];
	char* command;
	int codigoComando;

  //fclose(p->fConfiguracion);
  //p->fConfiguracion=AbrirArchivo("./server.conf");
  command=malloc(5);
  while(1)
  {
		fflush(stdout);
    bzero(buf,BUFSIZE);
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
