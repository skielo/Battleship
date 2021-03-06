#include "configuracion.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#define MAXLINE 80


/*
 * Funcion: AbrirArchivo
 * Objetivo: Abre un archivo de Configuracion En Modo de Solo Lectura
 *           y Texto y Devuelve un Descriptor De Archivo o NULL
 * 
 */
FILE* AbrirArchivo(const char* sNombre)
{
    return fopen(sNombre,"rt");
}


/*
 * Funcion: LeerValor
 * Objetivo: Lee de un archivo de configuracion el valor correspondiente a la clave ingresada
 *           si se encuentra el valor devuelve 1 y almacena en sResultado el valor correspondiente
 *           si no lo encuentra devuele -1
 * 
 */
int LeerValor(FILE* pArchivo, const char* sClave, char* sResultado)
{
	char linea[MAXLINE];
	char* clave;
	int largoClave;
  rewind(pArchivo);
  while(!feof(pArchivo))
  {
		bzero(linea,MAXLINE);
		fgets(linea,MAXLINE,pArchivo);   
		clave=(char*)malloc(sizeof(char)*MAXLINE);
		bzero(clave,MAXLINE);
		clave=strtok(linea,"=");
		if(strcmp(clave,sClave)==0)
		{
			largoClave=strlen(clave)+1;
			strcpy(clave,linea+largoClave);
			bzero(clave+strlen(clave),MAXLINE-strlen(clave));
			strcpy(sResultado,clave);
			return 0;
		}
	}
	return 1;   
}

