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
//char input_command[50];
//char file_name[50];
char ftp_input[101];
//given values
char *success_message = "yes";
char *fail_message = "no";
char *message_to_send = "ftp";
//char ip_addr[14] = "128.100.13.   ";




int main(int argc, char *argv[]){
    //error casting input
   if (argc != 3){
        printf("Try again with 2 arguments\n");
        exit(0);
    }
    /*GETTING THE IP ADDRESS*/
    char *ip_addr;
    //Case 1:  ug machine number is 2 digits
    if (strlen(argv[1]) == 21)
    {
        ip_addr = (char*)malloc(13*sizeof(char));
        //always same
        ip_addr[0] = '1';
        ip_addr[1] = '2';
        ip_addr[2] = '8';
        ip_addr[3] = '.';
        ip_addr[4] = '1';
        ip_addr[5] = '0';
        ip_addr[6] = '0';
        ip_addr[7] = '.';
        ip_addr[8] = '1';
        ip_addr[9] = '3';
        ip_addr[10] = '.';
        //machine number
        ip_addr[11] = argv[1][2];
        ip_addr[12] = argv[1][3];
        //end of string
        ip_addr[13] = '\0';
    }
    //Case 2:  ug machine number is 3 digits
    else if (strlen(argv[1]) == 22)
    {
        ip_addr = (char*)malloc(14*sizeof(char));
        //always same
        ip_addr[0] = '1';
        ip_addr[1] = '2';
        ip_addr[2] = '8';
        ip_addr[3] = '.';
        ip_addr[4] = '1';
        ip_addr[5] = '0';
        ip_addr[6] = '0';
        ip_addr[7] = '.';
        ip_addr[8] = '1';
        ip_addr[9] = '3';
        ip_addr[10] = '.';
        //machine number
        ip_addr[11] = argv[1][2];
        ip_addr[12] = argv[1][3];
        ip_addr[13] = argv[1][4];
        //end of string
        ip_addr[14] = '\0';
    }
    //else obviously wrong
    else
    {
        printf("Invalid server address\n");
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
    //serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    if (inet_addr(ip_addr) == -1)
    {
        printf("Invalid server address\n");
        exit(0);
    }
    else serv_addr.sin_addr.s_addr = inet_addr(ip_addr);

    /*Wait for server?*/

    printf("enter: ftp <filename>\n");
    //scanf("%s %s", input_command, file_name);
    //char *ftp_input;
    if (fgets(ftp_input, 60, stdin) == NULL)
    {
        printf("invalid input\n");
        exit(0);
    }
    char *input_command = strtok(ftp_input,  " ");
    char *file_name = strtok(NULL, "\n");
    if (input_command == NULL || file_name == NULL)
    {
        printf("invalid input\n");
        exit(0);
    }

    // make sure command is file transfer program
    if(strcmp(input_command, "ftp")!=0){
        printf("invalid command. remember: ftp\n");
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

    printf("waiting for server...\n");

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
        printf("A file transfer can start.\n");
    } else{
        exit(1);
    }

    close(sock_fd);
    return 1;
}