#include <stdio.h>
#include <string.h>

void send_http(char* host, char* msg, char* resp, size_t len);


/*
  Implement a program that takes a host, verb, and path and
  prints the contents of the response from the request
  represented by that request.
 */
int main(int argc, char* argv[]) 
{
  if (argc != 4)
  {
    printf("Invalid arguments - %s <host> <GET|POST> <path>\n", argv[0]);
    return -1;
  }
  char* host = argv[1];
  char* verb = argv[2];
  char* path = argv[3];

  /*
    STUDENT CODE HERE
   */

  char response[4096];
  char msg[1024];
  if (strcmp(verb,"GET")==0 || strcmp (verb,"POST")==0)
  {
    sprintf(msg, "%s %s HTTP/1.0\r\nHost:%s %s", verb, path, host, "\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/104.0.0.0 Safari/537.36\r\n\r\n");
    send_http(host, msg, response, 4096);
    printf("%s\n", response);
  }
// -------------
// char* f = strcat(e, "\r\n");
// printf("Overall name: %s\r\n", rcpt);
// printf("Overall statement: %s\r\n", recepient);
// send_smtp(socket, f, response, 1024);
// send_smtp(socket, "DATA\r\n", response, 1024);
// char* a = strcat("From:ksubram@iu.edu\r\nSubject:lab_2\r\nTo:", rcpt);
// char* b = strcat(a,"\r\n");
// char* c = strcat(b,filepath);
// char* d = strcat(c,"\r\n.\r\n");
// printf("Overall statement: %s\r\n", d);
// send_smtp(socket, d, response, 4096);
// printf("%s\n", response);
// send_smtp(socket, "QUIT\r\n", response, 1024);
  return 0;
}
