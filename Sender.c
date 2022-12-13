#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#define PORT 9999
#define MYFILE "test"
void main(){
    int my_Socket; // the socket
    char buffer[BUFSIZ]; // the file text
    char * com[1]; // exit message
    char size[256];// save file size
    int partSize1 = 0;//first part size
    int partSize2 = 0;//second part size
    int auth = 4190 ^ 3826;// xor
    int myFile;// file
    struct stat fileStat;// information of the file
    int sent = 0;// sent remaning size
    off_t offset = 0;//offset of the second part
    char buf[16]; // xor char
    sprintf(buf,"%d",auth);// cans the xor to char 
    struct sockaddr_in client_Address;
    bzero(&client_Address,sizeof(client_Address));//reset client
    client_Address.sin_family = AF_INET; // Address family, AF_INET unsigned 
    client_Address.sin_port = htons(PORT); // Port number 
    
    my_Socket = socket(AF_INET,SOCK_STREAM,0);//create socket
    if(my_Socket == -1){// check if create succeed
        printf("Socket creation failed\n");
        exit(0);
    }
    else printf("Socket creation succeed\n");
    int con = connect(my_Socket, (struct sockaddr *)&client_Address, sizeof(client_Address));// connect to server
    if(con == -1){// check if connect succeed
        printf("connection failed\n");
        exit(0);
    }
    else printf("connection succeed\n");
    while(1){// infinity loop
        myFile = open(MYFILE,O_RDONLY);//open file
        if(myFile == -1){ // check if open succeed
        printf("Open file failed\n");
        fprintf(stderr, "Error opening file --> %s\n", strerror(errno));
        exit(0);
        }
        else("file opened\n");
        fstat(myFile, &fileStat);// get file stats
        sprintf(size, "%ld", fileStat.st_size);// cast file size from int to char array
        send( my_Socket, size, sizeof(size), 0); // send size file to the server
        partSize1= (atoi(size))/2; // calculate first part size (cast char to int)
        offset = partSize1+1; //get offset to the second part
        partSize2 = (atoi(size)) - partSize1;//calculate the second part size
         while (((sent = sendfile(my_Socket, myFile, 0, BUFSIZ)) > 0) && (partSize1 > 0))// send the first part to server
        {       
                partSize1 -= sent;
        } 
        recv(my_Socket,buffer,BUFSIZ,0); // wait ro the authentication message 
        if(atoi(buffer)== atoi(buf)){   // check if the authentication message is ok
            if(setsockopt(my_Socket,IPPROTO_TCP,TCP_CONGESTION,"reno",4)<0){ // change the cc algorithem
                printf("eror changing cc\n");
                exit(0);
            }
            else printf("change cc - reno\n");
        }
           while (((sent = sendfile(my_Socket, myFile, &offset, BUFSIZ)) > 0) && (partSize2 > 0))//send the second part to server   
        {
                partSize2 -= sent;
        } 
        int command;
        printf("Enter command: (1) -> Send again, (-1) -> exit \n");// check if to send again or exit
        scanf("%d", &command);
        if(command == -1){ // if exit
            // sprintf(com,"%d",command);
            // printf("bye bye");
            // send( my_Socket, com, sizeof(com), 0); // send exit message
            close(my_Socket);//close socket
            break;
        }
        if(setsockopt(my_Socket,IPPROTO_TCP,TCP_CONGESTION,"cubic",5)<0){ // change the cc algorithem
        printf("eror changing cc\n");
        exit(0);
        }
        else printf("change cc - cubic\n");

    }

}