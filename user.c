
stClient MakeClient(int Port, char *Ip, int boatTable[10][10],  FILE *fLog)
{
    stClient retval;
    long ipAdress;
		int playTable[10][10];

    memset(&retval,(unsigned char)Port, 1);
    ipAdress=inet_addr(Ip);
    retval.ucIPaddress=(int)ipAdress;
    retval.iBoatTable=iBoatTable;
    retval.iPlayTable=playTable;
    
    /*Log(Log_Header_Pong,fLog,""); */
    
    return retval;
}

int ReadClient(FILE *fLog, stClient * client,LISTACLIENT * Lista, int iSocket)
{
		return 0;
}
