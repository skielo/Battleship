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
	char sOponente[10];	
	int hundido;
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
void IniciarJuegoConWorker(char *,char *, FILE *);
void EnviarInicioJuegoCliente(int, FILE *);
void MarcarClienteJugando(int );
void EnviarPlayCliente(int , char *, FILE* );
void RecibirJugadaCliente(int ,char *, FILE*);
void EnviarANSCliente(int , FILE*);

