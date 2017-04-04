#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

struct
{
    float dataBuf[4096/2][4096];
}__attribute__((packed)) _dataBuf;

int main()
{
    int udpSocket, nBytes;
    char buffer[16];
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size, client_addr_size;
    int i;

    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverStorage;

    // float indexVal = 0.00;
    srand((unsigned int)time(NULL));
    for(size_t i=0; i<4096; i++)
    {
    	for(size_t j=0; j<4096; j++)
    	{
    		// /a for [0, a]
    		_dataBuf.dataBuf[i][j] = (float)rand()/(float)(RAND_MAX);
        	// _dataBuf.dataBuf[i][j] = indexVal;
        	// indexVal += 1.00;
    	}
    }

    while(1){
        /* Try to receive any incoming UDP datagram. Address and port of
      requesting client will be stored on serverStorage variable */
        nBytes = recvfrom(udpSocket,buffer,16,0,(struct sockaddr *)&serverStorage, &addr_size);

        /*Convert message received to uppercase*/
        for(i=0;i<nBytes-1;i++)
            buffer[i] = toupper(buffer[i]);
        if(strcmp("GO", buffer))
            sendto(udpSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverStorage,addr_size);
    }

    return 0;
}
