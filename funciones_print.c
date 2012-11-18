#include "funciones_print.h"

#define NUMBER_X 10
#define NUMBER_Y 10

int i, j;

void print_map_line(char value[])
{
	printf("| ");
	for(j=0;j<NUMBER_Y;j++) {
		if (value[j]=='a') {
			printf(".");
		} else {
			printf("x");
		}
			printf(" ");
	}
	printf("|");
}

void print_header() 
{
	printf("+");
	for(i=0;i<43;i++) {
		printf("-");
	}
	printf("+");
}
void print_maps(char iBoatTable[][10], char iPlayTable[][10]) 
{
	printf("\t\t My map \t\t\t\t\t Remote Map\t\n");
	print_header();
	printf("\t");
	print_header();
	printf("\n");
	for(i=0;i<NUMBER_X;i++) {
		print_map_line(iBoatTable[i]);
		printf("\t");
		print_map_line(iPlayTable[i]);
		printf("\n");
	}
	print_header();
	printf("\t");
	print_header();
	printf("\n");
	fflush(stdout);
}

void MostrarClienteLista(char * sNombre, int iSock, char * sDireccionIP, int sPuerto, int iJugando) 
{
																				
	printf("------------------------------");
	printf("|\n");
	printf("|\tnombre: %s |\n",sNombre);
	printf("|\tsock: %d |\n",iSock);
	printf("|\tdire IP: %s |\n",sDireccionIP);
	printf("|\tPuerto: %d |\n",sPuerto);
	if(iJugando==1)
		printf("|\tjugando: SI |\n");
	else
		printf("|\tjugando: NO |\n");
	printf("|\n");
	printf("------------------------------\n");
	fflush(stdout);
}
