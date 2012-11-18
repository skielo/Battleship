#include "log.h"
#include "user.h"

int i,j;

void matrix_init(char * matrix, stClient * client)
{
	for(i=0;i<NUMBER_X;i++) {
		for(j=0;j<NUMBER_Y;j++) {
			client->iBoatTable[i][j] = 'a';
			client->iPlayTable[i][j] = 'a';
		}
	}
/*
	int x,y,i;
	for(i=1;i<7;i++) 
	{
		client->iBoatTable[matrix[i][0]-48][matrix[i][1]-48] = 'x';
	}

	for(i=2;i<6;i++)
	{
		printf("sin & x=%s\n",my_matrix[i]);
		x = atoi(&my_matrix[i][0]);
		y = atoi(&my_matrix[i][1]);
		client->iBoatTable[x][y] = 'x';
		printf("guardando en  x=%d, y=%d\n",x,y);
	}
*/
}

stClient MakeClient(char * nombre, char boatTable[],  FILE *fLog)
{
  stClient retval;
	strcpy(retval.sNombre,nombre);
	matrix_init(boatTable,&retval);	   
  Log(LOG_MENSAJE_EXTRA,fLog,"Nueva estructura cliente creada");
  
  return retval;
}

int ReadClient(FILE *fLog, stClient * client,LISTACLIENT * Lista, int iSocket)
{
		return 0;
}
