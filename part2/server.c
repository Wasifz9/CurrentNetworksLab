#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include<sys/socket.h>
#include <sys/types.h>
#include<arpa/inet.h>
#include <unistd.h> 
char *success_message = "yes";
char *fail_message = "no";
char *expected_client_message = "ftp";

char buffer[1024];

int main(int argc, char *argv[]){
    //error casting input
    if (argc != 2){
        printf("Try again with 1 argument.");
        exit(0);
    }

    //INET makes it IPv4 and DGRAM chooses UPD
    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    //socket() returns -1 on failure 
    if (sock_fd == -1){
        printf("No connection");
        exit(1);
    }


    //convert argument from string to integer and store
    int listen_port = atoi(argv[1]); 

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr)); //make sure struct is empty
    //initialize the values in the sockaddr_in struct
    serv_addr.sin_family = AF_INET; 
    //Finally, the sin_port must be in Network Byte Order (by using htons()!) (from Text)
    serv_addr.sin_port = htons(listen_port);
    //then ina.sin_addr.s_addr references the 4-byte IP address (in Network Byte Order) (from Text)
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);	
    // dont need cause of first memset but just to be sure
    memset(serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));

    //now attempt to BIND()
    if (bind(sock_fd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1 ){
        printf("couldn't bind\n");
        exit(1);
    }
    struct sockaddr_in client_addr;
    int length = sizeof(client_addr);
    //receiving the sockaddr from client and storing in client addr and its length in length
    if (recvfrom(sock_fd, buffer, 1024, 0, (struct sockaddr *) &client_addr, &length) == -1){
        printf("couldnt receive\n");
        exit(1);
    }



    if (strcmp(expected_client_message, buffer) == 0){//success
        if ((sendto(sock_fd, success_message, strlen(success_message), 0, (struct sockaddr *) &client_addr, length)) == -1) {
            printf("couldnt send\n");
            exit(1);
        }
    } else { //failure
        printf("wrong client message!!\n");
       if ((sendto(sock_fd, fail_message, strlen(fail_message), 0, (struct sockaddr *) &client_addr, length)) == -1) {
            printf("couldnt send\n");
            exit(1);
       }
    }
    
    close(sock_fd);
    return 0;

}