#include<stdio.h>

typedef struct Parametros{
  int sockClient;
  char * sDireccionIP;
	char * sPuerto;
	FILE * fConfiguracion;	
}stParam;

void LimpiarCRLF(char *);
void Mayusculas(char *);
int GenerarPuerto(int );
void * ConexionControl(void *);
stParam ArmarParametros(int ,const char *,const char *,FILE *);
int EvaluarComando(char *);
void Quit(int );
void ComandoInvalido(int );
