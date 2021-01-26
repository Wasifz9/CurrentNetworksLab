#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include<sys/socket.h>
#include <sys/types.h>
#include<arpa/inet.h>
#include <unistd.h> 
//max size and buffer initializations 
#define MAXSIZE 1024
char buffer[MAXSIZE];
char input_command[50];
char file_name[50];
//given values
char *success_message = "yes";
char *fail_message = "no";
char *message_to_send = "ftp";




int main(int argc, char *argv[]){
    //error casting input
   if (argc != 3){
        printf("Try again with 2 arguments");
        exit(0);
    }

    //convert argument from string to integer and store
    int listen_port = atoi(argv[2]); 
    //INET makes it IPv4 and DGRAM chooses UPD
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    //socket() returns -1 on failure 
    if (sock_fd == -1){
        printf("No connection\n");
        exit(1);
    }


    //declare sock struct 
    struct sockaddr_in serv_addr;
    //make sure struct is empty
    memset(&serv_addr, 0, sizeof(serv_addr));
    //initialize the values in the sockaddr_in struct
    serv_addr.sin_family = AF_INET; 
    //Finally, the sin_port must be in Network Byte Order (by using htons()!) (from Text)
    serv_addr.sin_port = htons(listen_port);	
    // dont need cause of first memset but just to be sure
    memset(serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));
    //stors IP adress in the s_addr
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    
    /////////////////////////////////////////////////////////////////////
    ///////////////// Dealing with user input /////////////////////////// /* STILL NEED TO DO THIS PROPERLY */
    /////////////////////////////////////////////////////////////////////
    printf("enter: ftp <filename>\n");
    scanf("%s %s", input_command, file_name);


    // make sure command is file transfer program
    if(strcmp(input_command, "ftp")!=0){
        printf("invalid command, remember: ftp");
        return 0;
    }

    //checking if file name exists and IF it exists, send the message yes to the server 
    if(access(file_name, F_OK) == -1) { //fail
        printf("File \"%s\" doesn't exist.\n", file_name);
        exit(1);
    } else{ // send a ftp
        if (sendto(sock_fd, (const char *)message_to_send, strlen(message_to_send), 0,
        (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1){
            printf("send failed\n"); 
            exit(1);
        }
    } 


    //declare receiving struct and set its length for the recv function 
    struct sockaddr_in to_recieve;
    int length = sizeof(to_recieve);


    //receiving the yes or no but... no will never happen because error checking client side 
    if (recvfrom(sock_fd, buffer, 1024, 0, (struct sockaddr *) &to_recieve, &length) == -1){
        printf("couldnt receive\n");
        exit(1);
    }


    //if get yes from server then bless 
    if (strcmp("yes", buffer) == 0){
        printf("A file transfer can start\n");
    } else{
        exit(1);
    }

    close(sock_fd);
    return 1;
}