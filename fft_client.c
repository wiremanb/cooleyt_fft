#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

//#define BUFSIZE 4096/2
#define BUFSIZE 512
#define COLUMNS 4096

struct
{
    unsigned char header;
    float dataBuf[COLUMNS];
//    float dataBuf[BUFSIZE][COLUMNS];
}__attribute__((packed)) _dataBuf;

int main()
{
    int clientSocket, portNum, nBytes;
    unsigned char header = 0xAA;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    /*Create UDP socket*/
    clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    // Create socket timeout
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
        perror("Socket timeout error");

    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverAddr;

    while(1)
    {
        _dataBuf.header = 0xAA;
        sendto(clientSocket,&_dataBuf.header,sizeof(_dataBuf.header),0,(struct sockaddr *)&serverAddr,addr_size);

        /*Receive message from server*/
        nBytes = recvfrom(clientSocket,&_dataBuf,sizeof(_dataBuf),0,NULL, NULL);

        // Received something from the server
        if(nBytes > 0 && _dataBuf.header == 0xBB)
        {
            for(size_t j=0; j<COLUMNS; j++)
                printf("Client data: %.02f\n", _dataBuf.dataBuf[j]);
        }
    }

    return 0;
}
