#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <time.h>
#include <errno.h>
#include <unistd.h> // read(), write(), close()
#define PORT 9999
#define NAME "newTest.txt"
//#define TIMELEN 20
void listenTo(int client_Socket){
    int index = 0; // printe times loop index
    char buffer[BUFSIZ];// the message from the client
    char sizefile[256]; // save the file size in buffer 
    int size =0;// save the cast of sizefile to integer
    double avarageTime=0; // save the avarage time of each part
    int partSize1 = 0; // save part1 size
    int partSize2 = 0; // save part2 size
    int auth = 4190 ^ 3826; // XOR
    FILE *fileRecived;// file recivef
    size_t length = 0; // length of func recv
    char buf[16]; // save char of xor
    char fin[5]; //save the finish
    clock_t time; // save recive message time
    sprintf(buf,"%d",auth); // cast the xor to char buffer
    size_t fileIndex = 0 ; //save file index
    double *part1times = calloc(1, sizeof(double));//dynamic array for times of first part
    double *part2times = calloc(1, sizeof(double));//dynamic array for times of second part



  
    for(; ;) // infinity loop
    {   

        recv(client_Socket,sizefile,256,0); // recivre message from client and save in buffer
        size = atoi(sizefile); // cast from char to int
        if(size == 0){
            printf("eror recive size/exit massege");
        }
        if(size == -2) { // if client want to exit
            printf("cubic:\n");
            while(index < fileIndex){ 
                avarageTime+=part1times[index]; // clculate all the time of the first parts
                printf("%f \n",part1times[index]); // print all the times of the first part
                index++;
            }
            avarageTime = avarageTime/(fileIndex); // calculate the avarage time of first parts
            printf("The avarage time is: %f \n",avarageTime); // prints the avarage times
            index =0; // reset 
            avarageTime=0; //reset
            printf("reno:\n");
            while( index < fileIndex){ 
                avarageTime+=part2times[index];// clculate all the time of the second parts
                printf("%f \n",part2times[index]);// print all the times of the second part
                index++;
            }
            avarageTime = avarageTime/(fileIndex);// calculate the avarage time of second parts
            printf("The avarage time is: %f \n",avarageTime);  // prints the avarage times
            fclose(fileRecived); // close the file
            break;
        }
        fileRecived = fopen(NAME, "w"); // open new file  
        if(fileRecived == NULL) // check if open succeed
        {
            printf("open file - failed\n");
            exit(0);
        }
        else printf("file crated\n");
        partSize1 = size/2; //calculate the first part size
        partSize2 = size - partSize1;//calculate the second part size
        time = clock();// save start recive time
        while((partSize1>0)){// get first part of file 
            length = recv(client_Socket,buffer,BUFSIZ,0);// recive data
            if(length == -1){
                printf("eror - recive data");
                exit(0);
            }
            if(fwrite(buffer, sizeof(char), length, fileRecived) == -1){// write to the new file
                printf("eror - write on file");
                exit(0);
            } 
            partSize1 -= length;

        }
        time = clock() - time; // save recived time
        part1times[fileIndex] = ((double)time)/CLOCKS_PER_SEC;; // save the time in seconds
        send(client_Socket, buf, 16,0);// send XOR to client
        if(setsockopt(client_Socket,IPPROTO_TCP,TCP_CONGESTION,"reno",4)<0){ // change the cc algorithem
                printf("eror changing cc\n");
                exit(0);
            }
        else printf("change cc - reno\n");
        time = clock(); // save start recive time
        while((partSize2>0)){ // get second part of file
            length = recv(client_Socket,buffer,BUFSIZ,0);//recive data
            if(length == -1){
                printf("eror - recive data");
                exit(0);
            }
            if(fwrite(buffer, sizeof(char), length, fileRecived) == -1){ // write to the file
                printf("eror - write data");
                exit(0);
            }
            partSize2 -= length;


        }
        time = clock() - time; // save recived time
        part2times[fileIndex] +=((double)time)/CLOCKS_PER_SEC;// save time in second
        sprintf(fin,"%d",3); // cast the fin to char buffer    
        send(client_Socket, fin, 5,0);// send finish to client
        fileIndex++; // inc the file index
        part1times = realloc(part1times,(fileIndex+1)*sizeof(double));//size++
        part2times = realloc(part2times,(fileIndex+1)*sizeof(double));//size++

        if(setsockopt(client_Socket,IPPROTO_TCP,TCP_CONGESTION,"cubic",5)<0){ // change the cc algorithem
                printf("eror changing cc\n");
                exit(0);
            }
        else printf("change cc - cubic\n");
    }
    free(part1times);
    free(part2times);
    close(client_Socket); // close client socken when finish
    }  

int main(){
    int mySocket,data, len, clientSocket; 
    struct in_addr serveraddr;
    struct sockaddr_in serverAddress, clientAddress;
    mySocket = socket(AF_INET,SOCK_STREAM,0);// crate socket
    if(mySocket < 0 ){ // check if socket crated succeed
        printf("Socket failed\n");
        exit(0);
    }
    else printf("Socket created\n");
    bzero(&serverAddress,sizeof(serverAddress)); //reset server address
    serverAddress.sin_family = AF_INET; // Address family, AF_INET unsigned 
    serverAddress.sin_port = htons(PORT); // Port number 
    serverAddress.sin_addr.s_addr =  INADDR_ANY;//ip address
    int yes=1; 
    if (setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) // set socket
    { 
    perror("setsockopt\n"); 
	exit(1);
    }
    if(bind( mySocket,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1) // be ready to connection
    {
        fprintf(stderr, "Error on bind --> %s", strerror(errno));
        exit(0);
    }
    else printf("Bind succeed\n");


    if((listen(mySocket, 1)) != 0){ // listen to connection
        fprintf(stderr, "Error on listen --> %s", strerror(errno));
        exit(0);
    }
    else printf("Listen succeed\n");
    len =sizeof(clientAddress); // get size of client 
    clientSocket = accept(mySocket,(struct sockaddr *)&clientAddress, &len); // save connection with client
    if( clientSocket < 0){ // chaeck if accept succeed
        printf("accept failed/n");
        exit(0);
    }
    else printf("eccept succeed\n");
    listenTo(clientSocket); // go to listenTo
    close(mySocket); // close socket
    return 0;
}
