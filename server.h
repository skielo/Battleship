#include<stdio.h>
#include "user.h"

typedef struct Parametros{
  int sockClient;
  char * sDireccionIP;
	char * sPuerto;
	FILE * fLog;	
}stParam;

struct shared_use_st {
	int used;
	char sUser[10];
	char mensaje[10];
	char sDireccion[25];
	int iPuerto;
}; 

void LimpiarCRLF(char *);
void Mayusculas(char *);
int GenerarPuerto(int );
void * ConexionControl(void *);
stParam ArmarParametros(int ,char *,char *, FILE *);
int EvaluarComando(char *);
void Quit(int );
void ComandoInvalido(int );
//void itoa(int , char[]);
void reverse(char []);
int InsertarListaConexion( int , NODOClient *, LISTACLIENT *, int , char * );
int ContarListaConexion();
void EnviarListaDeUsuarios(int , FILE*);
void ManejarConexionConectada(int , FILE * );
int maximoValor(int[]);
void MostrarLista(LISTACLIENT );
int EstaEnLista(char *);
void IniciarJuegoConWorker(char *, FILE *);
void EnviarInicioJuegoCliente(int, FILE *);
