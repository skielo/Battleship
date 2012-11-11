#include<stdio.h>
#include "user.h"

typedef struct Parametros{
  int sockClient;
  char * sDireccionIP;
	char * sPuerto;
	FILE * fLog;	
}stParam;

void LimpiarCRLF(char *);
void Mayusculas(char *);
int GenerarPuerto(int );
void * ConexionControl(void *);
stParam ArmarParametros(int ,char *,char *, FILE *);
int EvaluarComando(char *);
void Quit(int );
void ComandoInvalido(int );
void itoa(int , char[]);
void reverse(char []);
int InsertarListaConexion( int , stClient * , ListasClient *);
int CrearConexionDeControl(char *, char *);
