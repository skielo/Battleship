#include "log.h"
#include <sys/types.h>
#include <unistd.h>

FILE* ArchivoLog (char* Nombre){
    FILE* archlog;

    if((archlog=fopen((const char*)Nombre,"at"))==NULL){
        printf("Error al abrir el archivo");
        exit (EXIT_FAILURE);
       }
    return archlog;
}

void Log(int TipoEvento,FILE *archlog,const char* sMensaje){
    time_t tiempoActual;
    struct tm *pTiempo;
    char *fecha;

    tiempoActual=time(NULL); /*Se obtiene el tiempo actual*/
    pTiempo=gmtime(&tiempoActual);
    fecha=asctime(pTiempo); /*Convierte la fecha y tiempo actual en un detereminado formato*/
    fecha[strlen(fecha)-1]='\0';
    fprintf(archlog,"%s : ",fecha);
    fprintf(archlog,"PID: %d | ",(int)getpid());
        
        switch(TipoEvento){
            case 1:
                    fprintf(archlog,"Conexión de cliente.\n ");
                    break;
            case 2:
                    fprintf(archlog,"Mensaje %s.\n",sMensaje);
                    break;
            case 3:
                    fprintf(archlog,"Error %s.\n",sMensaje);
                    break;
            case 4:
                    fprintf(archlog,"Cierre del socket de escucha.\n");
                    break;
            case 5:
                    fprintf(archlog,"Cierre del socket de conexion.\n");
                    break;
            case 6:
                    fprintf(archlog,"Peticion del archivo %s.\n",sMensaje);
                    break;
            case 7:
                    fprintf(archlog,"Archivo %s enviado.\n",sMensaje);
                    break;
	    			case 8:
                    fprintf(archlog,"Archivo %s recibido.\n",sMensaje);    
                    break;
            case 9:
                    fprintf(archlog,"El usuario %s no se conecto correctamente.\n",sMensaje);
                    break;
            case 10:
                    fprintf(archlog,"El usuario %s se conecto al sistema.\n",sMensaje);
                    break;
            case -1:
                    fprintf(archlog,"%s",sMensaje);
                    break;
            
        }
}


