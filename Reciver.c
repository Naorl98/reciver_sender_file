#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <unistd.h> // read(), write(), close()
#define PORT 9999
#define NAME "newTest.txt"
#define TIMELEN 20
void listenTo(int client_Socket){
    int len;
    static double part1times[TIMELEN] = {0}; // times of part one
    static double part2times[TIMELEN] = {0};// times of part two
    int index = 0; // printe times loop index
    char buffer[BUFSIZ];// the message from the client
    int size =0;
    double avarageTime=0; // save the avarage time of each part
    int partSize1 = 0; // save part1 size
    int partSize2 = 0; // save part2 size
    int auth = 4190 ^ 3826; // XOR
    FILE *fileRecived;// file recivef
    ssize_t length; // length of func recv
    char * buf[16]; // save char of xor
    clock_t time; // save recive message time
    sprintf(buf,"%d",auth); // cast the xor to char budder
    int timeIndex = 0 ; //save times index
    
    for(; ;) // infinity loop
    {
        bzero(buffer, BUFSIZ); //reset buffer
        recv(client_Socket,buffer,BUFSIZ,0); // recivre message from client and save in buffer
        size = atoi(buffer); // cast from char to int
        if(size == -2) { // if client want to exit
            index=0;
            printf("cubic:\n");
            while(part1times[index] != 0 && index != TIMELEN){ 
                avarageTime+=part1times[index]; // clculate all the time of the first parts
                printf("%f \n",part1times[index]); // print all the times of the first part
                index++;
            }
            avarageTime = avarageTime/(index+1); // calculate the avarage time of first parts
            printf("The avarage time is: %f \n",avarageTime); // prints the avarage times
            index =0; // reset 
            avarageTime=0; //reset
            printf("reno:\n");
            while(part2times[index]!= 0 && index != TIMELEN){ 
                avarageTime+=part2times[index];// clculate all the time of the second parts
                printf("%f \n",part2times[index]);// print all the times of the second part
                index++;
            }
            avarageTime = avarageTime/(index+1);// calculate the avarage time of second parts
            printf("The avarage time is: %f \n",avarageTime);  // prints the avarage times
            fclose(fileRecived); // close the file
            break;

        }
        partSize1 = size/2; //calculate the first part size
        partSize2 = size - partSize1;//calculate the second part size
        
        fileRecived = fopen(NAME, "w"); // open new file
        
        if(fileRecived == NULL) // check if open succeed
        {
            printf("open file - failed\n");
            exit(0);
        }
        time = clock();
        while((partSize1>0)&&((length = recv(client_Socket,buffer,BUFSIZ,0))>0)){
            fwrite(buffer, sizeof(char), length, fileRecived);
            partSize1 -= length;
        }
        time = clock() - time;
        part1times[timeIndex] = ((double)time)/CLOCKS_PER_SEC;;
        write(client_Socket, buf, 16);
        time = clock();
        while((partSize2>0)&&((length = recv(client_Socket,buffer,BUFSIZ,0))>0)){
            fwrite(buffer, sizeof(char), length, fileRecived);
            partSize2 -= length;
        }
        time = clock() - time;
        part2times[timeIndex] +=((double)time)/CLOCKS_PER_SEC;
        timeIndex++;
    }
    close(client_Socket);
    
}

void main(){
    int mySocket,data, len, clientSocket;
    struct in_addr serveraddr;
    struct sockaddr_in serverAddress, clientAddress;
    mySocket = socket(AF_INET,SOCK_STREAM,0);
    if(mySocket < 0 ){
        printf("Socket failed\n");
        exit(0);
    }
    else printf("Socket created\n");
    bzero(&serverAddress,sizeof(serverAddress));
    serverAddress.sin_family = AF_INET; // Address family, AF_INET unsigned 
    serverAddress.sin_port = htons(PORT); // Port number 
    serverAddress.sin_addr.s_addr =  INADDR_ANY;
    int yes=1; 
    if (setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
    { 
    perror("setsockopt\n"); 
	exit(1);
    }
    if(bind( mySocket,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        fprintf(stderr, "Error on bind --> %s", strerror(errno));
        exit(0);
    }
    else printf("Bind succeed\n");


    if((listen(mySocket, 1)) != 0){
        fprintf(stderr, "Error on listen --> %s", strerror(errno));
        exit(0);
    }
    else printf("Listen succeed\n");
    len =sizeof(clientAddress);
    clientSocket = accept(mySocket,(struct sockaddr *)&clientAddress, &len);
    if( clientSocket < 0){
        printf("accept failed/n");
        exit(0);
    }
    else printf("eccept succeed\n");
    listenTo(clientSocket);
    close(mySocket);

}