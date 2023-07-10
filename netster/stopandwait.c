#include "file.h"
#include "chat.h"
#include <stdio.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/poll.h>

#define MAXSIZE 255
#define MAX_CLIENT 100
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))


/*
 *  Here is the starting point for your netster part.3 definitions. Add the 
 *  appropriate comment header as defined in the code formatting guidelines
 */

typedef struct rudp_s
{
    int seq;
    int ack;
    unsigned char packet[MAXSIZE];
}rudp;

// Used polling help from StackOverflow
// https://stackoverflow.com/questions/2876024/linux-is-there-a-read-or-recv-from-socket-with-timeout

/* Add function definitions */
// Command: /netster -i 0.0.0.0 -p 8085 -f ass6
// ./netster -i 127.0.0.1 -p 8085 -f test_new.txt -r 1
void stopandwait_server(char* iface, long port, FILE* fp) {

    struct addrinfo *result, *rp;
    // unsigned char client_file[6]={0};
    char message_256[MAXSIZE+1] = {0};
    struct sockaddr_in address;
    struct addrinfo hints;
    // int sockfd, opt=1;
    int sockfd;
    // int addrlen = sizeof(address), new_socket=0; 
    int readval=0;
    char cport[5]={0};
    // size_t total=0;

    sprintf(cport, "%ld", port);
    bzero(&address, sizeof(address));
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_UDP;
    // Source: getaddrinfo man page
    int s = getaddrinfo(iface, cport, &hints, &result);
    if(s != 0)
    {
        // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }
    // Resolving the address identified from byte to text format
    // Source: Computer Networking Github Wiki
    for(rp=result; rp!=NULL; rp=rp->ai_next)
    {
        if (rp->ai_family == AF_INET) 
        { // Address is IPv4
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = htonl(INADDR_ANY);
            address.sin_port = htons(port);
        }
        else if(rp->ai_family == AF_INET6)
        {
            address.sin_family = AF_INET6;
            address.sin_addr.s_addr = htonl(INADDR_ANY);
            address.sin_port = htons(port);
        }
    }
    if ((sockfd = socket(address.sin_family, SOCK_DGRAM, 0))<0)
    {
        // printf("Error with UDP socket creation\n");
        exit(1);
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 50; // 50 ms
    // if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    if(setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO | SO_RCVTIMEO, &timeout, sizeof(timeout)))
    {
        // printf("setsockopt\n");
        exit(1);
    }
    if (bind(sockfd, (struct sockaddr*) &address, sizeof(address)) < 0)
    {
        // printf("Bind operation failed\n");
        exit(1);
    }
    char satisifed = '1', rdp_fsm = '0';
    struct sockaddr_in client_udp_address;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000000/1000;
    struct timeval stop, start;
    // int timeout = 0;// No(0), Yes(1); 1 - means resend packet; 0 - means read new file data
    gettimeofday(&start, NULL); //Starting timer
    int sent = 0;
    while(1)
    {
        bzero(&client_udp_address, sizeof(client_udp_address));
        unsigned int client_length = sizeof(client_udp_address); 
        if(sent == 1)
        {
            printf("Entered sent1\n");
            gettimeofday(&stop, NULL);
            if(((((stop.tv_sec - start.tv_sec) * 1000) + (stop.tv_usec - start.tv_usec))/1000) >= tv.tv_usec)
                {
                    printf("Start Time: %lu %lu\n", start.tv_sec, start.tv_usec);
                    printf("Stop Time: %lu %lu\n", stop.tv_sec, stop.tv_usec);
                    printf("Set Time: %lu\n", tv.tv_usec);
                    printf("Time difference: %lu\n", (((stop.tv_sec - start.tv_sec) * 1000) + (stop.tv_usec - start.tv_usec))/1000);
                    break;
                    exit(0);
                }
        }
        printf("Waiting for packet from client\n");
        readval = recvfrom(sockfd, (void*)message_256, MAXSIZE, MSG_WAITALL, (struct sockaddr*) &client_udp_address, &client_length);
        if(readval <=0)
        {
            // if(sent == 1)
            // {
            //     gettimeofday(&stop, NULL);
            //     if(((((stop.tv_sec - start.tv_sec) * 1000000) + (stop.tv_usec - start.tv_usec))/1000) >= tv.tv_usec)
            //     {
            //         break;
            //         exit(0);
            //     }
            // }
            break;
            exit(0);
        }
        if(satisifed == '1') //Unimportant varible and condition
        {
            if(rdp_fsm == '0') //Inside state 0,i.e. seq 0
            {
                printf("entering rdp fsm 0\n");
                if(message_256[0] == '0') //if received packet seq is 0
                {
                    printf("entering fsm 0\n");
                    /*
                    * Packet corruption integrity (checksum) is taken care by UDP protocol itself.
                    * So, once correct sequence is received, writing that to a file. 
                    */
                    size_t ret = fwrite(&message_256[2], 1, readval-2, fp);
                    bzero(message_256, MAXSIZE+1);
                    /*
                    * rewriting seq and ack packet to be sent back to the client for verification.
                    */
                    message_256[0]='0';
                    message_256[1]='0';
                    rdp_fsm = '1'; //Moving to the next finite-state-machine, i.e. seq 1
                    if(sendto(sockfd, message_256, 2, MSG_CONFIRM, (const struct sockaddr*) &client_udp_address, client_length)  == -1)
                    {
                        exit(1);
                    }
                    gettimeofday(&start, NULL);
                    sent = 1;
                    if (ret <= 0) 
                    {
                        exit(EXIT_FAILURE);
                    }
                    bzero(message_256, MAXSIZE+1);
                    printf("exiting fsm 0\n");
                }
                else
                {
                    printf("exiting rdp fsm 0\n");
                    continue;
                }
            }
            else if(rdp_fsm == '1')
            {
                printf("entering rdp fsm 1\n");
                if(message_256[0] == '1')
                {
                    printf("entering fsm 1\n");
                    size_t ret = fwrite(&message_256[2], 1, readval-2, fp);
                    bzero(message_256, MAXSIZE+1);
                    message_256[0]='1';
                    message_256[1]='0';
                    rdp_fsm = '0';
                    if(sendto(sockfd, message_256, 2, MSG_CONFIRM, (const struct sockaddr*) &client_udp_address, client_length)  == -1)
                    {
                        exit(1);
                    }
                    gettimeofday(&start, NULL);
                    sent = 1;
                    if (ret <= 0) 
                    {
                        exit(EXIT_FAILURE);
                    }
                    bzero(message_256, MAXSIZE+1);
                    printf("exiting fsm 1\n");
                }
                else
                {
                    printf("exiting rdp fsm 1\n");
                    continue;
                }
            }
            
        }
    }
    close(sockfd);
    exit(0);
}


// Command: /netster 0.0.0.0 -p 8085 -f snapghost.png
// ./netster 127.0.0.1 -p 8085 -f test.txt -r 1
void stopandwait_client(char* host, long port, FILE* fp) 
{
    int sockfd=0; 
    // int sockfd_client; 
    size_t total=0;
    struct sockaddr_in address;
    struct addrinfo *result, *rp;
    struct addrinfo hints;
    char cport[5]={0};
    char dst[MAXSIZE];
    // unsigned char message[5]={0};
    char message_253[MAXSIZE-2+1]={0};
    void* raw_addr;

    sprintf(cport, "%ld", port);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;
    

    // Source: getaddrinfo man page
    int s = getaddrinfo(host, cport, &hints, &result);
    if(s != 0)
    {
        // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }
    

    // Checking the iface entered is IPv4 or IPv6
    for(rp=result; rp!=NULL; rp=rp->ai_next)
    {
        if (rp->ai_family == AF_INET) 
        { // Address is IPv4
            struct sockaddr_in* tmp = (struct sockaddr_in*)rp->ai_addr;
            raw_addr = &(tmp->sin_addr);
            inet_ntop(rp->ai_family, raw_addr, dst, 100);
            address.sin_family = AF_INET;
            address.sin_port = htons(port);
        }
        else if(rp->ai_family == AF_INET6)
        {
            struct sockaddr_in* tmp = (struct sockaddr_in*)rp->ai_addr;
            raw_addr = &(tmp->sin_addr);
            inet_ntop(rp->ai_family, raw_addr, dst, 100);
            address.sin_family = AF_INET6;
            address.sin_port = htons(port);
        }
    }

    if ((sockfd = socket(address.sin_family, SOCK_DGRAM, 0))<0)
    {
        exit(1);
    }

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(address.sin_family, dst, &address.sin_addr)<= 0) 
    {
        exit(0);
    }
    char message_255[MAXSIZE+1]={0};
    message_255[0]='0';
    message_255[1]='0';
    char rdt_fsm = (char) message_255[0];
    int satisfied = 1; //1(YES), 0(NO)
    int counter = 0;

    /* Setting up a timer for 50 millisecond
    * capture the timer before sending a packet from system time
    * and then get the system time for every wait and compare with 50 millisecond
    */
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 50000/1000;
    // tv.tv_usec = 50000; //microsec debug for timeout situations
    struct timeval stop, start;
    int timeout = 0;// No(0), Yes(1); 1 - means resend packet; 0 - means read new file data
    gettimeofday(&start, NULL); //Starting timer
    size_t ret=0;
    while(1)
    {
        // Set satisfied=1, so that it works the first time
        counter++;
        if(satisfied == 1)
        {
            
            if(timeout == 0)
            {
                // File is read
                bzero(message_253, sizeof(message_253));
                ret = fread(message_253, sizeof(*message_253), MAXSIZE-2, fp);
                if(ret <= 0 )
                {
                    total = total + ret;
                    bzero(message_253, sizeof(message_253));
                    sendto(sockfd, message_253, 0, MSG_CONFIRM, (const struct sockaddr*) &address, sizeof(address));
                    sendto(sockfd, message_253, 0, MSG_CONFIRM, (const struct sockaddr*) &address, sizeof(address));
                    sendto(sockfd, message_253, 0, MSG_CONFIRM, (const struct sockaddr*) &address, sizeof(address));
                    sendto(sockfd, message_253, 0, MSG_CONFIRM, (const struct sockaddr*) &address, sizeof(address));
                    sendto(sockfd, message_253, 0, MSG_CONFIRM, (const struct sockaddr*) &address, sizeof(address));
                    exit(0);
                }

                // Storing the read file data into the packet for transfer
                for(int i=0; i<=253; i++)
                {
                    message_255[i+2] = message_253[i];
                }
            }
            
            printf("seq: %c, ack: %c \n", message_255[0], message_255[1]);
            gettimeofday(&start, NULL); //Starting timer
            if(sendto(sockfd, message_255, ret+2, MSG_CONFIRM, (const struct sockaddr*) &address, sizeof(address))  == -1)
            {
                exit(1);
            }
            timeout = 0;
            satisfied = 0; //explicitly set to 0, so only activated when conditions are met.
        }
        
        if(rdt_fsm == '0') //Wait and loop until ACK for Seq 0 is received
        {
            printf("entering fsm 0\n");
            gettimeofday(&stop, NULL);
            printf("Start Time: %lu %lu\n", start.tv_sec, start.tv_usec);
            printf("Stop Time: %lu %lu\n", stop.tv_sec, stop.tv_usec);
            printf("Set Time: %lu\n", tv.tv_usec);
            printf("Time difference: %lu\n", (((stop.tv_sec - start.tv_sec) * 1000) + (stop.tv_usec - start.tv_usec))/1000);
            //debugging if statement
            // if(((((stop.tv_sec - start.tv_sec) * 1000000) + (stop.tv_usec - start.tv_usec))/1000) >= tv.tv_usec || counter == 3)
            if(((((stop.tv_sec - start.tv_sec) * 1000) + (stop.tv_usec - start.tv_usec))/1000) >= tv.tv_usec)
            {
                printf("timeout happened\n");
                satisfied = 1;
                timeout = 1;// Timeset occured, resend the same packet
                continue;
            }
            unsigned int a = sizeof(address);
            
            printf("waiting for response from server\n");
            struct pollfd fd;
            int tm_codn;
            int readval;
            fd.fd = sockfd; 
            fd.events = POLLIN;
            tm_codn = poll(&fd, 1, 50); // 50 millisecond for timeout
            switch (tm_codn) 
            {
                case -1:
                    // Error
                    break;
                case 0:
                    // Timeout 
                    satisfied = 1;
                    timeout = 1;
                    break;
                default:
                    // recv(mySocket,buf,sizeof(buf), 0); // get your data
                    readval = recvfrom(sockfd, (void*)message_255, MAXSIZE, MSG_WAITALL, (struct sockaddr*) &address, &a);
                    break;
            }
            if(tm_codn == 0)
            {
                printf("timeout happend 1\n");
                continue;
            }
            // int readval = recvfrom(sockfd, (void*)message_255, MAXSIZE, MSG_WAITALL, (struct sockaddr*) &address, &a);
            if(readval <=0 )
            {
                printf("readval <=0 happened\n");
                break;
                exit(0);
            }
            /*
            * Checking for corrupt or improper packet/bit handling.
            * Looping and waiting, if not.
            */
            if(message_255[0] != '0' && message_255[1] != '0')
            {
                
                rdt_fsm = '0';
                satisfied = 0;
                printf("contining 0\n");
                continue;
            }
            /*
            * When received ACK and correct SEQ from received,
            * variable 'satisfied' is flagged.
            * Doing so, client application reads the next part of the text file to be sent.
            */
            satisfied = 1;
            rdt_fsm = '1'; //Changing the seq from 0 to 1, when conditions are satisfied
            message_255[0] = '1';
            printf("exiting fsm 0\n");
        }
        else if(rdt_fsm == '1') //Alternating between seq 0 and 1. Same operations.
        {
            printf("entering fsm 1\n");
            gettimeofday(&stop, NULL);
            printf("Start Time: %lu %lu\n", start.tv_sec, start.tv_usec);
            printf("Stop Time: %lu %lu\n", stop.tv_sec, stop.tv_usec);
            printf("Set Time: %lu\n", tv.tv_usec);
            printf("Time difference: %lu\n", (((stop.tv_sec - start.tv_sec) * 1000) + (stop.tv_usec - start.tv_usec))/1000);
            //debugging if statement below
            // if(((((stop.tv_sec - start.tv_sec) * 1000000) + (stop.tv_usec - start.tv_usec))/1000) >= tv.tv_usec || counter == 3) 
            if(((((stop.tv_sec - start.tv_sec) * 1000) + (stop.tv_usec - start.tv_usec))/1000) >= tv.tv_usec)
            {
                printf("timeout happened\n");
                satisfied = 1;
                timeout = 1;// Timeset occured, resend the same packet
                continue;
            }
            unsigned int a = sizeof(address);
            printf("waiting for response from server\n");
            struct pollfd fd;
            int tm_codn;
            int readval;
            fd.fd = sockfd; // your socket handler 
            fd.events = POLLIN;
            tm_codn = poll(&fd, 1, 50); // 50 millisecond for timeout
            switch (tm_codn) 
            {
                case -1:
                    // Error
                    break;
                case 0:
                    // Timeout 
                    satisfied = 1;
                    timeout = 1;
                    break;
                default:
                    // recv(mySocket,buf,sizeof(buf), 0); // get your data
                    readval = recvfrom(sockfd, (void*)message_255, MAXSIZE, MSG_WAITALL, (struct sockaddr*) &address, &a);
                    break;
            }
            if(tm_codn == 0)
            {
                printf("timeout happend 1\n");
                continue;
            }
            // int readval = recvfrom(sockfd, (void*)message_255, MAXSIZE, MSG_WAITALL, (struct sockaddr*) &address, &a);
            
            if(readval <=0 )
            {
                printf("readval <=0 happened\n");
                break;
                exit(0);
            }
            if(message_255[0] != '1' && message_255[1] != '0')
            {
                printf("contining 0\n");
                rdt_fsm = '1';
                satisfied = 0;
                continue;
            }
            satisfied = 1;
            message_255[0] = '0';
            rdt_fsm = '0';
            printf("exiting fsm 1\n");
        }
    } 
    close(sockfd);
    exit(0);
}
