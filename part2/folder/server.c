#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include<sys/socket.h>
#include <sys/types.h>
#include<arpa/inet.h>
#include <unistd.h> 
#include <regex.h>
char *success_message = "yes";
char *fail_message = "no";
char *expected_client_message = "ftp";
#define BUF_SIZE 1000

char buffer[1024];
//Packet Struct
struct Packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char file_data[1000];
};

void parsePacket(char *packet, char **total_frag, char **frag_no, 
                char **size, char **filename, char **filedata) 
{
    char *token = strtok(packet, ":"); 
    int c = 0;

    while (token != NULL) { 
        switch (c) {
            case 0:
                *total_frag = token;
            case 1:
                *frag_no = token;
            case 2:
                *size = token;
            case 3:
                *filename = token;
            case 4:
                *filedata = token; 
        }
        c += 1;
        token = strtok(NULL, ":"); 
    }
}


void stringToPacket(const char* str, struct Packet *packet) { ///// not my code must be changed or the parsing must be done differently.

    // Compile Regex to match ":"
    regex_t regex;
    if(regcomp(&regex, "[:]", REG_EXTENDED)) {
        fprintf(stderr, "Could not compile regex\n");
    }

    // Match regex to find ":" 
    regmatch_t pmatch[1];
    int cursor = 0;
    char buf[1000];

    // Match total_frag
    if(regexec(&regex, str + cursor, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0, BUF_SIZE * sizeof(char));
    memcpy(buf, str + cursor, pmatch[0].rm_so);
    packet -> total_frag = atoi(buf);
    cursor += (pmatch[0].rm_so + 1);

    // Match frag_no
    if(regexec(&regex, str + cursor, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0,  BUF_SIZE * sizeof(char));
    memcpy(buf, str + cursor, pmatch[0].rm_so);
    packet -> frag_no = atoi(buf);
    cursor += (pmatch[0].rm_so + 1);

    // Match size
    if(regexec(&regex, str + cursor, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }
    memset(buf, 0, BUF_SIZE * sizeof(char));
    memcpy(buf, str + cursor, pmatch[0].rm_so);
    packet -> size = atoi(buf);
    cursor += (pmatch[0].rm_so + 1);

    // Match filename
    if(regexec(&regex, str + cursor, 1, pmatch, REG_NOTBOL)) {
        fprintf(stderr, "Error matching regex\n");
        exit(1);
    }


    memcpy(packet -> filename, str + cursor, pmatch[0].rm_so);
    packet -> filename[pmatch[0].rm_so] = 0;
    cursor += (pmatch[0].rm_so + 1);
    
    // Match filedata
    memcpy(packet -> file_data, str + cursor, packet -> size);

    // printf("total_frag:\t%d\n", packet -> total_frag);
    // printf("frag_no:\t%d\n", packet -> frag_no);
    // printf("size:\t%d\n", packet -> size);
    // printf("filename:\t%s\n", packet -> filename);
    // printf("filedata:\t%s\n", packet -> filedata);

}
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

    int pack_num = 0;
    int packs_to_rec = 100;
    FILE *outStream = NULL;
    char *total_frag, *frag_no, *size, *filename, *filedata;
    while (pack_num < packs_to_rec){
        struct Packet new_packet;
        if (recvfrom(sock_fd, buffer, 1024, 0, (struct sockaddr *) &client_addr, &length) == -1){
            printf("couldnt receive\n");
            exit(1);
        }
        //ONE WAY TI DO IT
        /*stringToPacket(buffer, &new_packet);

        //parse_packet(buffer, &new_packet);
        if (new_packet.frag_no == 1){
            outStream = fopen("servercopy1", "w");
        }

        //fputs(packet.file_data, outStream);
        fwrite(new_packet.file_data, sizeof(char), new_packet.size, outStream);
        
        pack_num = new_packet.frag_no;
        packs_to_rec = new_packet.total_frag;*/


        parsePacket(&buffer, &total_frag, &frag_no,
            &size, &filename, &filedata);
        
        printf("Packet %s : \n %s \n", frag_no, filedata);
        //get total and current packet
        long totalPackets = strtol(total_frag, NULL, 10);
        long currentPacket = strtol(frag_no, NULL, 10);
        int dataSize = atoi(size);
        if (currentPacket == 1){
            outStream = fopen("servercopy1", "w");
        }
        fwrite(filedata, sizeof(char), dataSize, outStream);
        //get size of packets filedata

        pack_num = currentPacket;
        packs_to_rec = totalPackets;
    }
    
    close(sock_fd);
    return 0;

}