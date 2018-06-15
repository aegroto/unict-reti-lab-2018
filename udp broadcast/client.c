/****************************************************************/
/**                  Sample UDP broadcast project              **/
/**                         Client Side                        **/
/** Full source code: https://github.com/aegroto/udp-broadcast **/
/****************************************************************/

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 

int main(int argc, char**argv) {
  int sockfd, n;
  struct sockaddr_in remote_addr;
  char sendline[1000];
  char recvline[1000];
  socklen_t len = sizeof(remote_addr);
   
  if (argc != 3) { 
    printf("usage:  UDPclient <remote_IP remote_port>\n");
    return 1;
  }

  if((sockfd=socket(AF_INET,SOCK_DGRAM,0)) < 0) { 
    printf("\nErrore nell'apertura del socket");
    return -1;
  }
  
  memset(&remote_addr,0,sizeof(remote_addr));
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_addr.s_addr=inet_addr(argv[1]);
  remote_addr.sin_port=htons(atoi(argv[2]));
  
  sendto(sockfd, "hello! i'm new\n", 15, 0, (struct sockaddr *) &remote_addr, sizeof(remote_addr));

  int pid = fork();

  if(pid == 0) { 
    while(1) {      
      n=recvfrom(sockfd,recvline,1000,0, (struct sockaddr *) &remote_addr, &len);
      recvline[n]=0;
      printf("From IP:%s Port:%d msg: %s", inet_ntoa(remote_addr.sin_addr),  ntohs(remote_addr.sin_port), recvline);
    }
  } else {
    while (fgets(sendline, 1000,stdin) != NULL) {
      sendto(sockfd,sendline,strlen(sendline),0,
            (struct sockaddr *) &remote_addr, sizeof(remote_addr));
    }
  }
}
