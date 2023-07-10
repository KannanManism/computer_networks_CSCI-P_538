#include <stdio.h>
#include <string.h>

int connect_smtp(const char* host, int port);
void send_smtp(int sock, const char* msg, char* resp, size_t len);



/*
  Use the provided 'connect_smtp' and 'send_smtp' functions
  to connect to the "lunar.open.sice.indian.edu" smtp relay
  and send the commands to write emails as described in the
  assignment wiki.
 */
int main(int argc, char* argv[]) {
  // if (argc != 3) {
  //   printf("Invalid arguments - %s <email-to> <email-filepath>", argv[0]);
  //   return -1;
  // }

  // char* rcpt = argv[1];
  // char* filepath = argv[2];
  // fscanf(fptr, "%[^\n]", email_data);
  /* 
     STUDENT CODE HERE
   */

int socket = connect_smtp("lunar.open.sice.indiana.edu", 25);

char response[1024];
send_smtp(socket, "HELO iu.edu\r\n", response, 1024);
send_smtp(socket, "MAIL FROM: ksubram@iu.edu\r\n", response, 1024);
send_smtp(socket, "RCPT TO: ksubram@iu.edu\r\n", response, 1024);
send_smtp(socket, "DATA\r\n", response, 1024);
send_smtp(socket, "From:ksubram@iu.edu\r\nSubject:lab_2\r\nTo:ksubram@iu.edu\r\nHello World\r\n.\r\n", response, 1024);
printf("%s\n", response);
send_smtp(socket, "QUIT\r\n", response, 1024);

return 0;
}
