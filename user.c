#include "log.h"
#include "user.h"
#include <stdlib.h>

int i,j;

void matrix_init(char my_matrix[][NUMBER_Y], stClient * client)
{
	for(i=0;i<NUMBER_X;i++) {
		for(j=0;j<NUMBER_Y;j++) {
			client->iBoatTable[i][j] = 'a';
			client->iPlayTable[i][j] = 'a';
		}
	}
	
	for(i=2;i<=6;i++)
	{
		client->iBoatTable[atoi(my_matrix[i][0])][atoi(my_matrix[i][1])] = 'x';
	}
}
/*
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
void print_maps() 
{
	printf("\t\t My map \t\t\t\t\t Remote Map\t\n");
	print_header();
	printf("\t");
	print_header();
	printf("\n");
	for(i=0;i<NUMBER_X;i++) {
		print_map_line(my_matrix[i]);
		printf("\t");
		print_map_line(remote_matrix[i]);
		printf("\n");
	}
	print_header();
	printf("\t");
	print_header();
	printf("\n");
}
*/
stClient MakeClient(char * nombre, char boatTable[][10],  FILE *fLog)
{
  stClient retval;
	retval.sNombre=nombre;
/*
		for(i=0;i<10;i++)
		{
			for(j=0;j<10;j++)
			{
				retval.iBoatTable[i][j]=boatTable[i][j];
			}
		}
 */
	matrix_init(boatTable,&retval);	   
  Log(LOG_MENSAJE_EXTRA,fLog,"Nueva estructura cliente creada");
  
  return retval;
}

int ReadClient(FILE *fLog, stClient * client,LISTACLIENT * Lista, int iSocket)
{
		return 0;
}
