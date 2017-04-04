#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE 512
#define COLUMNS 4096

struct
{
<<<<<<< HEAD
    float dataBuf[4096/2][4096];
=======
    unsigned char header;
    float dataBuf[COLUMNS];
//    float dataBuf[BUFSIZE][COLUMNS];
>>>>>>> 83b4b0656e3a7791bb1b2c2f7c69a7542f8d98c0
}__attribute__((packed)) _dataBuf;

int main()
{
    int udpSocket, nBytes;
    unsigned char header;
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
    while(1)
    {
        nBytes = recvfrom(udpSocket,&_dataBuf.header,sizeof(_dataBuf.header),0,(struct sockaddr *)&serverStorage, &addr_size);

        if(_dataBuf.header == 0xAA && nBytes > 0)
        {
            _dataBuf.header = 0xBB;
            for(size_t j=0; j<COLUMNS; j++)
            {
                _dataBuf.dataBuf[j] = (float)rand()/(float)(RAND_MAX);
                printf("Server data: %.02f\n", _dataBuf.dataBuf[j]);
            }
            sendto(udpSocket,&_dataBuf,sizeof(_dataBuf),0,(struct sockaddr *)&serverStorage,addr_size);
        }
    }

    return 0;
}
