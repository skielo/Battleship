#include "log.h"
#include "user.h"

int i,j;

void matrix_init(char my_matrix[][NUMBER_Y], stClient * client)
{
	for(i=0;i<NUMBER_X;i++) {
		for(j=0;j<NUMBER_Y;j++) {
			client->iBoatTable[i][j] = 'a';
			client->iPlayTable[i][j] = 'a';
		}
	}

	int x,y,i;
	for(i=2;i<6;i++)
	{
		x = (my_matrix[i][0])-48;
		y = (my_matrix[i][1])-48;
		client->iBoatTable[x][y] = 'x';
	}
}

stClient MakeClient(char * nombre, char boatTable[][10],  FILE *fLog)
{
  stClient retval;
	retval.sNombre=nombre;
	matrix_init(boatTable,&retval);	   
  Log(LOG_MENSAJE_EXTRA,fLog,"Nueva estructura cliente creada");
  
  return retval;
}

int ReadClient(FILE *fLog, stClient * client,LISTACLIENT * Lista, int iSocket)
{
		return 0;
}
