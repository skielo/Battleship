#include "funciones_red.h"
/*
    Crea un socket y verifica si hubo error en la creacion
*/
int MakeSocket(int dominio, int tiposocket, int flags)
{
    int retval;
    
    retval=socket(dominio,tiposocket,flags);

    return retval;
}
/*
    Conecta un socket a una direccion y puerto, y verifica error
*/
int ConnectSocket(int socketToConnect, struct sockaddr *addrToConnect, socklen_t addrSize)
{
    int retval;
    retval=connect(socketToConnect,addrToConnect,addrSize);
    return retval;
    	   
}

/*
    Enlaza un socket a una direccion y puerto, y verifica error
*/
int BindSocket(int socketToBind, struct sockaddr *addrToBind, socklen_t addrSize)
{
    return bind(socketToBind,addrToBind,addrSize);
}

/*
    Pone a un socket a escuchar una cantidad determinada de conexiones encolables
    y chequea error.  
*/
int ListenSocket(int socketToListen,int conexionesEncolables)
{
    return listen(socketToListen,conexionesEncolables);
}

/*
    Acepta una conexion entrante y devuelve un socket para su uso, chequea errores
*/
int AcceptSocket(int socketListening, struct sockaddr *addrListening, socklen_t *addrSize)
{
    int retval;
    
    retval=accept(socketListening, addrListening, addrSize);
    
    
    return retval;

}

/*
    Cierra un socket y chequea error
*/
int CloseSocket(int socketToClose)
{
    return close(socketToClose);
}

/*
    Lee datos de un socket y devuelve la cantidad de bytes leidos, chequea error
    
*/
ssize_t ReadSocket(int socketToRead, void *buffer, size_t bufferLenght, int flags)
{
    ssize_t retval;
    
    retval = recv(socketToRead, buffer, bufferLenght, flags);
    if(retval==-1 && errno==EAGAIN)
    { 
      perror("recv");
      return 0;
    }

    return retval;
}

/*
    Escribe datos de un socket y devuelve la cantidad de bytes escritos, chequea error
    
*/
ssize_t WriteSocket(int socketToRead, void *buffer, size_t bufferLenght, int flags)
{
    ssize_t retval;
    
    retval = send(socketToRead, buffer, bufferLenght, flags);

    return retval;
}

void AllowMultipleConection(int socketToListen)
{
	int opt = 1;
	if(setsockopt(socketToListen, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
}

