#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include<sys/socket.h>
#include <sys/types.h>
#include<arpa/inet.h>
#include <unistd.h> 
#include <time.h> 
//max size and buffer initializations 
#define MAXSIZE 1024
#define BUF_SIZE 1100
char buffer[MAXSIZE];
//char input_command[50];
//char file_name[50];
char ftp_input[101];
//given values
char *success_message = "yes";
char *fail_message = "no";
char *message_to_send = "ftp";
struct timeval end_time, start_time;

//char ip_addr[14] = "128.100.13.   ";
//Packet Struct
struct Packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char file_data[1000];
};

char *concat(char const*str1, char const*str2) {
   size_t const l1 = strlen(str1) ;
   size_t const l2 = strlen(str2) ;

    char* result = malloc(l1 + l2 + 1);
    if(!result) return result;
    memcpy(result, str1, l1) ;
    memcpy(result + l1, str2, l2 + 1);
    return result;
}


void printPacket(struct Packet p) {
    printf("\n-- Packet --\n");
    printf("total_frag = %d | frag_no = %d | size = %d | filename = %s\n", 
        p.total_frag, p.frag_no, p.size, p.filename);
    printf("data: %s", p.file_data);
}

void packetToString(const struct Packet *packet, char *serializedPacket) { // not my code must be changed or done differently.
    
    // Initialize string buffer
    memset(serializedPacket, 0, BUF_SIZE);

    // Load data into string
    int cursor = 0;
    sprintf(serializedPacket, "%d", packet -> total_frag);
    cursor = strlen(serializedPacket);
    memcpy(serializedPacket + cursor, ":", sizeof(char));
    ++cursor;
    
    sprintf(serializedPacket + cursor, "%d", packet -> frag_no);
    cursor = strlen(serializedPacket);
    memcpy(serializedPacket + cursor, ":", sizeof(char));
    ++cursor;

    sprintf(serializedPacket + cursor, "%d", packet -> size);
    cursor = strlen(serializedPacket);
    memcpy(serializedPacket + cursor, ":", sizeof(char));
    ++cursor;

    sprintf(serializedPacket + cursor, "%s", packet -> filename);
    cursor += strlen(packet -> filename);
    memcpy(serializedPacket + cursor, ":", sizeof(char));
    ++cursor;

    memcpy(serializedPacket + cursor, packet -> file_data, sizeof(char) * 1000);

    // printf("%s\n", packet -> filedata);
    // printf("Length of data: %d\n", strlen(packet -> filedata));

}

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
    } else{ ///// open, read and partition file into packets         
        if (sendto(sock_fd, (const char *)message_to_send, strlen(message_to_send), 0,
        (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1){
            printf("send failed\n"); 
            exit(1);
        }
        gettimeofday(&start_time, NULL); //start time after send 
    }

    printf("waiting for server...\n");

    //declare receiving struct and set its length for the recv function 
    struct sockaddr_in to_recieve;
    int length = sizeof(to_recieve);


    //receiving the yes or no but... no will never happen because error checking client side 
    if (recvfrom(sock_fd, buffer, 1024, 0, (struct sockaddr *) &to_recieve, &length) == -1){
        printf("couldnt receive\n");
        exit(1);
    } else{
        gettimeofday(&end_time, NULL);
    }


    //if get yes from server then bless 
    if (strcmp("yes", buffer) == 0){
        printf("A file transfer can start.\n");
    } else{
        exit(1);
    }


    FILE * file_to_send = fopen(file_name, "r");
    int c;

    if(file_to_send == NULL) { //fail
        printf("File \"%s\" couldn't be opened.\n", file_name);
        exit(1);
    }
    
    //declare first packet
    struct Packet packet; 
    packet.filename = file_name;
    packet.frag_no = 1;
    
    //get total size of file
    fseek(file_to_send, 0L, SEEK_END); 
    int total_frag = ftell(file_to_send) / 1000 + 1;
    packet.total_frag = total_frag; 
    rewind(file_to_send);

    printf("Number of packets required: %d\n", total_frag);
    FILE *outStream = NULL;
    while (packet.frag_no <= packet.total_frag) {
       /* int size_count = 0;
        while(size_count<1000) {
            c = fgetc(file_to_send);
            if(feof(file_to_send)) { 
                break;
            }
            //printf("%c", c);
            packet.file_data[size_count] = c;
            size_count += 1;
        }*/

        fread((void*)packet.file_data, sizeof(char), 1000, file_to_send);
        /// form packet, send packet and get acknowledgement
        if (packet.frag_no < packet.total_frag){
            packet.size = sizeof(packet.file_data);
        } else{
            fseek(file_to_send, 0, SEEK_END);
            packet.size = (ftell(file_to_send) - 1) % 1000 + 1;
        }
        //packet.size = size_count;
        //serialize current packet
        char *serializedPacket = malloc(sizeof(char) * 1100); 
        asprintf(&serializedPacket, "%d:%d:%d:%s:", packet.total_frag, 
         packet.frag_no, packet.size, packet.filename);
        
        char *final = concat(serializedPacket, packet.file_data);
        //memmove(serializedPacket + sizeof(serializedPacket), packet.file_data, packet.size);
        //printPacket(packet);
        //send packet
        //packetToString(&packet, serializedPacket);

        sendto(sock_fd, (const char *)final, strlen(final), 
            0, (const struct sockaddr *) &serv_addr, sizeof(serv_addr));
    
        printf("Packet %d : \n %s \n", packet.frag_no, final);
        printf("Packet %d : \n %s \n", packet.frag_no, packet.file_data);

        if (packet.frag_no == 1){
            outStream = fopen("clientcopy1", "w");
        }

        //fputs(packet.file_data, outStream);
        fwrite(packet.file_data, sizeof(char), packet.size, outStream);
        //increment frag number working on and reset file data
        packet.frag_no += 1;
        memset(packet.file_data, 0, sizeof(packet.file_data));
    }


    /*char* data;
    fread((void*)data, 1, 70, file_to_send);
    printf("%s", data);*/









    unsigned long microsecond_rtt = (end_time.tv_sec - start_time.tv_sec) * 1000000 + end_time.tv_usec - start_time.tv_usec;
    printf("\nRTT took %lu microseconds.\n", microsecond_rtt);
    printf("%d characters.\n", packet.size);
    close(sock_fd);
    fclose(outStream);
    return 1;
}

/*void sendPacket (struct Packet packet) {

}

struct Packet createEmptyPacket (unsigned int total_frag, unsigned int frag_no, char* filename) {

}*/