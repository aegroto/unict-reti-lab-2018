/************************************************************/
/**                Sample UDP guard project                **/
/**                       Server Side                      **/
/** Full source code: https://github.com/aegroto/udp-guard **/
/**                  Released under GPLV3                  **/
/************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

char CMD_REQUEST[] = "/request_access\n";
char CMD_YIELD[] = "/yield_access\n";
char REP_NO_FILE[] = "NO";

#define YIELD_PORT atoi(argv[1])+1500

int main(int argc, char** argv) {
    int welcome_sockfd, yield_sockfd, n;
    extern int errno;
    struct sockaddr_in local_addr, remote_addr, yield_addr;
    socklen_t len = sizeof(remote_addr);
    char mesg[1000], rep_mesg[1000];
    char* filename = argv[2];
    srand(time(0));

    if(argc < 2) {
        printf("Use: server <listening_port file>");
        return 0;
    }	 

    // welcome socket

    if((welcome_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        printf("Socket open error: %s (%d)\n", strerror(errno), errno);
        return -1;
    }

    int opt_welcome_port_reuse;
    if (setsockopt(welcome_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &opt_welcome_port_reuse, sizeof(opt_welcome_port_reuse)) == -1){
        printf("Reuse port Error : %s (%d)\n", strerror(errno), errno);
        return errno;
    }

    int opt_welcome_addr_reuse = 1;
    if (setsockopt(welcome_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &opt_welcome_addr_reuse, sizeof(opt_welcome_addr_reuse)) != 0) {
        printf("Reuse addr error: %s (%d)\n", strerror(errno), errno);
        return errno;
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(atoi(argv[1]));

    if(bind(welcome_sockfd, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0) {
        printf("Binding error: %s (%d) \n", strerror(errno), errno);
        return -1;
    }

    // yield socket

    if((yield_sockfd=socket(AF_INET,SOCK_DGRAM,0)) < 0) { 
        printf("Socket open error: %s (%d)\n", strerror(errno), errno);
        return -1;
    }

    int opt_yield_port_reuse;
    if (setsockopt(yield_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &opt_yield_port_reuse, sizeof(opt_yield_port_reuse)) != 0){
        printf("Reuse port Error : %s (%d)\n", strerror(errno), errno);
        return errno;
    }

    int opt_yield_addr_reuse = 1;
    if (setsockopt(yield_sockfd, SOL_SOCKET, SO_REUSEADDR, (char*) &opt_yield_addr_reuse, sizeof(opt_yield_addr_reuse)) != 0) {
        printf("Reuse addr error: %s (%d)\n", strerror(errno), errno);
        return errno;
    }

    memset(&yield_addr, 0, sizeof(yield_addr));
    yield_addr.sin_family = AF_INET;
    yield_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    yield_addr.sin_port = htons(YIELD_PORT);

    if(bind(yield_sockfd, (struct sockaddr *) &yield_addr, sizeof(yield_addr)) < 0) {
        printf("Binding error: %s (%d) \n", strerror(errno), errno);
        return -1;
    }

    while(1) {
        // listening to welcome port
        printf("Listening to welcome port...\n");
        n = recvfrom(welcome_sockfd, mesg, 1000, 0, (struct sockaddr *) &remote_addr, &len);
        mesg[n] = 0;

        printf("Received message: %sParsing...\n", mesg);

        if(strncmp(mesg, "FILE=", 5) == 0) { // a client is connecting and is asking for a specific file
            printf("A client is requesting for a specific file (%s), i'm guarding %s. Checking...\n", mesg+5, filename);

            if(strcmp(filename, (mesg+5)) == 0) {
                sendto(welcome_sockfd, "OK", 2, 0, (struct sockaddr *) &remote_addr, sizeof(remote_addr));
            } else {
                sendto(welcome_sockfd, REP_NO_FILE, strlen(REP_NO_FILE), 0, (struct sockaddr *) &remote_addr, sizeof(remote_addr));
            }
        } else if(strcmp(mesg, CMD_REQUEST) == 0) { // a client that's already connected is asking for access to the resource
            printf("A client has requested access, accepting...\n");

            sprintf(rep_mesg, "%d", YIELD_PORT);
            sendto(welcome_sockfd, rep_mesg, strlen(rep_mesg), 0, (struct sockaddr *) &remote_addr, sizeof(remote_addr));
            
            // listen to messages in the yield port
            printf("Waiting for the yield message...\n");
            n = recvfrom(yield_sockfd, mesg, 1000, 0, (struct sockaddr *) &remote_addr, &len);
            mesg[n] = 0;
            printf("Received message: %sParsing...\n", mesg);

            if(strcmp(mesg, CMD_YIELD) == 0) {
                /*if(bind(welcome_sockfd, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0) {
                    printf("Binding error: %s (%d) \n", strerror(errno), errno);
                    return -1;
                }*/

                printf("Resource yielded.\n");
            }
        }
    }
}