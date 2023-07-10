/*
  Kannan Mani Subramanian Maniarasusekar,
  ksubram@iu.edu
  Computer Networking, 2022, 
  Indiana University Bloomington
*/

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#define BUF_SIZE 500


/*
  Use the `getaddrinfo` and `inet_ntop` functions to convert a string host and
  integer port into a string dotted ip address and port.
 */
int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Invalid arguments - %s <host> <port>", argv[0]);
    return -1;
  }

  char* host = argv[1];
  char* port = argv[2];

  /*
    STUDENT CODE HERE
   */

  /* Use raw_addr as a generic address for inet_ntop */

   /*
      Code snippets gathered from class github wiki,
      and from Linux man pages.
    */
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  struct addrinfo *result, *rp;
  /* Utilized the code snippet from the Wiki suggested man page. */
  int s = getaddrinfo(host, port, &hints, &result);
  if(s != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    exit(EXIT_FAILURE);
  }
  
  /*
    Code snippets gathered from class github wiki,
    and from Linux man pages.
  */
  char dst[BUF_SIZE];
  for(rp=result; rp!=NULL; rp=rp->ai_next)
  {
    void* raw_addr;
      if (rp->ai_family == AF_INET) 
      { // Address is IPv4

        struct sockaddr_in* tmp = (struct sockaddr_in*)rp->ai_addr;
        raw_addr = &(tmp->sin_addr);
        inet_ntop(rp->ai_family, raw_addr, dst, rp->ai_addrlen);
        printf("IPv4 %s\n", dst);

      }
      else if(rp->ai_family == AF_INET6)
      {
        struct sockaddr_in6* tmp = (struct sockaddr_in6*) rp->ai_addr;
        raw_addr = &(tmp->sin6_addr);
        inet_ntop(rp->ai_family, raw_addr, dst, rp->ai_addrlen);
        printf("IPv6 %s\n", dst);
      }
      
  }
  freeaddrinfo(result);
  freeaddrinfo(rp);
  return 0;
}