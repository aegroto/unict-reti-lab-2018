/************************************************************/
/**                Sample UDP guard project                **/
/**                       Client Side                      **/
/** Full source code: https://github.com/aegroto/udp-guard **/
/**                  Released under GPLV3                  **/
/************************************************************/


#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define STATE_IDLE 0
#define STATE_WAIT 1
#define STATE_ACCESS 2

char CMD_REQUEST[] = "/request_access\n";
char CMD_YIELD[] = "/yield_access\n";
char REP_NO_FILE[] = "NO";
// char REP_GRANTED[] = "access granted";

int init_socket(int argc, char** argv, int* sockfd, struct sockaddr_in* remote_addr, int port) {
    if(((*sockfd)=socket(AF_INET,SOCK_DGRAM,0)) < 0) { 
        printf("\nErrore nell'apertura del socket\n");
        return -1;
    }

    memset(remote_addr, 0, sizeof(*remote_addr));
    remote_addr->sin_family = AF_INET;
    remote_addr->sin_addr.s_addr = inet_addr(argv[1]);
    remote_addr->sin_port = htons(port);

    return 0;
}

int file_request(int argc, char** argv, int* sockfd, char* sendline, char* recvline, int* state, struct sockaddr_in* remote_addr, socklen_t* len) {
    // is the server guarding the right file?
    sprintf(sendline, "FILE=%s", argv[3]);
    sendto(*sockfd, sendline, strlen(sendline), 0, (struct sockaddr *) remote_addr, sizeof(*remote_addr));

    // wait for response
    int n = recvfrom(*sockfd, recvline, 1000, 0, (struct sockaddr*) remote_addr, len);
    recvline[n] = 0;

    printf("Received response: %s\n", recvline);
    if(strcmp(recvline, REP_NO_FILE) == 0) {
        printf("The server is not guarding this file\n");
        return -1;
    }

    *state = STATE_IDLE;

    return 0;
}

int main(int argc, char** argv) {
    int sockfd, n;
    struct sockaddr_in remote_addr;
    char sendline[1000];
    char recvline[1000];
    char c;
    socklen_t len = sizeof(remote_addr);
    int state;
    int retcode = 0;
    FILE* file;

    if (argc != 4) { 
        printf("usage:  UDPclient <remote_IP remote_port file_path>\n");
        return 1;
    }

    retcode = init_socket(argc, argv, &sockfd, &remote_addr, atoi(argv[2]));
    if(retcode != 0) return retcode;

    retcode = file_request(argc, argv, &sockfd, sendline, recvline, &state, &remote_addr, &len);
    if(retcode != 0) return retcode;

    while (fgets(sendline, 1000, stdin) != NULL) {
        // command mode
        if(sendline[0] == '/') {
            if(strcmp(sendline, CMD_REQUEST) == 0 && state != STATE_ACCESS) {
            // request access
                state = STATE_WAIT;
                sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *) &remote_addr, sizeof(remote_addr));
                printf("Sent command: %s\n", sendline);

                // wait for response
                printf("Access request sent. Waiting for response...\n");
                n = recvfrom(sockfd, recvline, 1000, 0, (struct sockaddr *) &remote_addr, &len);
                recvline[n] = 0;
                printf("Received response: %s\n", recvline);

                state = STATE_ACCESS;

                retcode = init_socket(argc, argv, &sockfd, &remote_addr, atoi(recvline));
                if(retcode != 0) return retcode;

                file = fopen(argv[3], "a");
            } else if(strcmp(sendline, CMD_YIELD) == 0 && state == STATE_ACCESS) {
            // yield access
                fclose(file);
                sendto(sockfd, sendline, strlen(sendline), 0, (struct sockaddr *) &remote_addr, sizeof(remote_addr));
                printf("Sent command: %s\n", sendline);
                state = STATE_IDLE;
                
                retcode = init_socket(argc, argv, &sockfd, &remote_addr, atoi(argv[2]));
                if(retcode != 0) return retcode;
            } else {
            // unknown or invalid command
                printf("This command is invalid in the current state (%d)\n", state);
            }
        } else if (state == STATE_ACCESS) { 
        // otherwise we're trying to modify the resource
            fprintf(file, "%s", sendline);
        }
    }
}