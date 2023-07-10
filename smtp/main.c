#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int connect_smtp(const char* host, int port);
void send_smtp(int sock, const char* msg, char* resp, size_t len);



/*
  Use the provided 'connect_smtp' and 'send_smtp' functions
  to connect to the "lunar.open.sice.indian.edu" smtp relay
  and send the commands to write emails as described in the
  assignment wiki.
 */

/* 
  Utilized code from a website for reading string from a file
  Reference: https://www.codewithc.com/opening-reading-text-file-c/
*/
int main(int argc, char* argv[]) 
{
  if (argc != 3) {
    printf("Invalid arguments - %s <email-to> <email-filepath>", argv[0]);
    return -1;
  }

  char* rcpt = argv[1];
  char* filepath = argv[2];
  /* 
     STUDENT CODE HERE
   */

  // Reading the email.txt file
  FILE *fptr;
  char email_reading[1000], email_body[1000];
  fptr = fopen(filepath, "r");
  while (fgets(email_reading,1000, fptr)!=NULL)
  {
    sprintf(email_body, "%s",email_reading);
  }

  int socket = connect_smtp("lunar.open.sice.indiana.edu", 25);
  // Constructing the email header
  char response[4096];
  send_smtp(socket, "HELO iu.edu\r\n", response, 1024);
  char sender[100];
  sprintf(sender, "MAIL FROM: %s\r\n", rcpt);
  send_smtp(socket, sender, response, 1024);
  char recepient[100];
  sprintf(recepient, "RCPT TO: %s\r\n", rcpt);
  send_smtp(socket, recepient, response, 1024);
  send_smtp(socket, "DATA\r\n", response, 1024);
  char email[4096];
  sprintf(email, "From:%s\r\nSubject:lab_2\r\nTo:%s\r\n%s\r\n.\r\n",rcpt, rcpt, email_body);
  send_smtp(socket, email, response, 4096);
  printf("%s", response);

  // Ending the telnet connection and closing file
  send_smtp(socket, "QUIT\r\n", response, 1024);
  fclose(fptr);

  return 0;
}
