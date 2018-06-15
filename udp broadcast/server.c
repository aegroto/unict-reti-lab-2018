/****************************************************************/
/**                  Sample UDP broadcast project              **/
/**                         Server Side                        **/
/** Full source code: https://github.com/aegroto/udp-broadcast **/
/**                     Released under GPLV3                   **/
/****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAX_CLIENTS 10

int main(int argc, char**argv)
{
  int sockfd, n;
  extern int errno;
  struct sockaddr_in local_addr, remote_addr;
  socklen_t len = sizeof(remote_addr);
  char mesg[1000], broadcast_mesg[1100];
  
  struct sockaddr_in clients[MAX_CLIENTS];
  size_t first_client_slot = 0;
   
  if(argc < 2) {
    printf("Use: server listening_PORT");
	  return 0;
  }	 

  if((sockfd=socket(AF_INET,SOCK_DGRAM,0)) < 0) { 
    printf("\nErrore nell'apertura del socket");
    return -1;
  }

  memset(&local_addr,0,sizeof(local_addr));
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr=htonl(INADDR_ANY);
  local_addr.sin_port=htons(atoi(argv[1]));

  if(bind(sockfd, (struct sockaddr *) &local_addr, sizeof(local_addr)) < 0) {
    printf("\nErrore nel binding. Errore %d \n",errno);
    return -1;
  }

  while(1) { 
    n = recvfrom(sockfd, mesg, 1000, 0, (struct sockaddr *) &remote_addr, &len);
    mesg[n] = 0;
	  printf("From IP:%s Port:%d msg: %s", inet_ntoa(remote_addr.sin_addr),  ntohs(remote_addr.sin_port), mesg);

    if(first_client_slot < MAX_CLIENTS) {
      int saved_client = 0;

      for(size_t i = 0; i < first_client_slot; ++i) {
        if(clients[i].sin_addr.s_addr == remote_addr.sin_addr.s_addr && clients[i].sin_port == remote_addr.sin_port) {
          saved_client = 1;
          break;
        }
      }

      if(saved_client == 0) {
        clients[first_client_slot].sin_family = remote_addr.sin_family;
        clients[first_client_slot].sin_port = remote_addr.sin_port;
        clients[first_client_slot].sin_addr.s_addr = remote_addr.sin_addr.s_addr;

        ++first_client_slot;

        printf("[BROADCAST] Saved new client %s:%d\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
      }
    } else {
      printf("[BROADCAST] Could not save client %s:%d (not enough space)\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
    }
    printf("\n");

    // broadcast
    sprintf(broadcast_mesg, "[BROADCAST] Client %s:%d said: %s", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port), mesg);
    for(size_t i = 0; i < first_client_slot; ++i) {
	    sendto(sockfd, broadcast_mesg, strlen(broadcast_mesg), 0, (struct sockaddr *)&clients[i], len);
    }
  }
}
