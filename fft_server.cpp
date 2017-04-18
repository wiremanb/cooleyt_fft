#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#define COLUMNS 4096

struct PACKET {
    unsigned char header; //0xAA - New client connection
                          //0xBB - Server to client get file
                          //0xCC - Client to server return fft value
    char name[256];
    float fft;
}__attribute__((packed));

void FFT()
{

}

int main(int argc, char *argv[])
{
    int udpSocket, nBytes;
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage serverStorage;
    struct PACKET _dataBuf;
    socklen_t addr_size, client_addr_size;

    if(argc < 2)
    {
        printf("Suggest use: ./s <number of clients>");
        return -1;
    }
    int numberOfClients = atoi(argv[1]);
    //    unsigned char clientHeaders[numberOfClients]; // Create client header array size of how many clients will connect
    //    memset(&clientHeaders[0], NULL, numberOfClients);

    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(7891);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    // Create socket timeout
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    if (setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0)
        perror("Socket timeout error");

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverStorage;

    srand((unsigned int)time(NULL));
    static int clientCount = 0;
    while(1)
    {
        nBytes = recvfrom(udpSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverStorage, &addr_size);
        if(nBytes > 0 && _dataBuf.header == 0xAA && clientCount < numberOfClients)
        {
            printf("Client name: %s\n", _dataBuf.name);
            clientCount++;
            _dataBuf.header = 0xBB;
            strcpy(_dataBuf.name, "Server");
            sendto(udpSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverStorage,addr_size);
        }

        nBytes = recvfrom(udpSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverStorage, &addr_size);
        if(nBytes > 0 && _dataBuf.header == 0xCC)
        {
            printf("%s fft: %.02f", _dataBuf.name, _dataBuf.fft);
            FFT();
        }
    }

    return 0;
}
