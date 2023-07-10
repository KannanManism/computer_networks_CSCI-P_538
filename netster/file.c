#include "file.h"
#include "chat.h"
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#define MAXSIZE 256
#define MAX_CLIENT 100
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
/*
 *  Here is the starting point for your netster part.2 definitions. Add the 
 *  appropriate comment header as defined in the code formatting guidelines
 */

/* Add function definitions */
void file_server(char* iface, long port, int use_udp, FILE* fp) {
   
    struct addrinfo *result, *rp;
    unsigned char client_file[MAXSIZE]={0};
    struct sockaddr_in address;
    struct addrinfo hints;
    int sockfd, new_socket=0, opt=1, addrlen = sizeof(address), readval=0;
    char cport[5]={0};
    size_t total=0;

    sprintf(cport, "%ld", port);
    bzero(&address, sizeof(address));
    memset(&hints, 0, sizeof(hints));

    if(use_udp == 0)
    {    
        hints.ai_family = PF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        hints.ai_protocol = IPPROTO_TCP;
        // Source: getaddrinfo man page
        int s = getaddrinfo(iface, cport, &hints, &result);
        if(s != 0)
        {
            // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            // exit(EXIT_FAILURE);
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

        // Using AF_LOCAL caused "bind failed" 
        if ((sockfd = socket(address.sin_family, SOCK_STREAM, 0))<0)
        {
            //  printf("Error with TCP socket creation\n");
             exit(1);
        }
    }
    else
    {
        hints.ai_family = PF_UNSPEC;
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
    }

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        // printf("setsockopt\n");
        exit(1);
    }
    if (bind(sockfd, (struct sockaddr*) &address, sizeof(address)) < 0)
    {
        // printf("Bind operation failed\n");
        exit(1);
    }
    while(1)
    {
        if(use_udp == 0)
        {
            if(listen(sockfd, MAX_CLIENT) < 0)
            {
                // printf("Listen operation failed\n");
                exit(1);
            }

            if((new_socket = accept(sockfd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) <0)
            {
                // printf("Error with accept operation\n");
                exit(1);
            }
            else //Successfully socket created and client connected
            {
                while(1)
                {
                    readval = recv(new_socket, client_file, MAXSIZE, 0);
                    if(readval <=0 )
                    {
                        break;
                        return;
                    }
                    size_t ret = fwrite(client_file, 1, readval, fp);
                    // size_t ret = fwrite(client_file, 1, 1, fp);
                    total = (long unsigned int) total + (long unsigned int) ret;
                    // printf("Writing: %zu\n", total );
                    // fprintf(fp, "%s", client_file);
                    if (ret <= 0) 
                    {
                        // fprintf(stderr, "fwrite() failed: %zu\n", ret);
                        exit(EXIT_FAILURE);
                    }
                    bzero(client_file, sizeof(client_file));
                }   
                    close(new_socket);
                    return;
                
                // }
            } 
        

        } //End of TCP Server
        else // UDP Server
        {
            while(1)
            {
                //variables declaration
                // fp = fopen("recv.txt", "w");
                struct sockaddr_in client_udp_address;
                bzero(&client_udp_address, sizeof(client_udp_address));
                unsigned int client_length = sizeof(client_udp_address); 

                readval = recvfrom(sockfd, client_file, MAXSIZE, MSG_WAITALL, (struct sockaddr*) &client_udp_address, &client_length);
                if(readval <=0 )
                {
                    break;
                    return;
                }
                size_t ret = fwrite(client_file, 1, readval, fp);
                // size_t ret = fwrite(client_file, sizeof(*client_file), ARRAY_SIZE(client_file), fp);
                // if (ret != ARRAY_SIZE(client_file)) {
                // fprintf(stderr, "fwrite() failed: %zu\n", ret);
                // exit(EXIT_FAILURE);
                // }
                // // fprintf(fp, "%s", client_file);
                // bzero(client_file, sizeof(client_file));

                total = (long unsigned int) total + (long unsigned int) ret;
                // printf("Writing: %zu\n\r", total );
                // fprintf(fp, "%s", client_file);
                if (ret <= 0) 
                {
                    // fprintf(stderr, "fwrite() failed: %zu\n", ret);
                    exit(EXIT_FAILURE);
                }
                bzero(client_file, readval);
            }
            close(sockfd);
            return;
        } 
    }       
}

void file_client(char* host, long port, int use_udp, FILE* fp) {
    int sockfd=0, sockfd_client; 
    size_t total=0;
    struct sockaddr_in address;
    struct addrinfo *result, *rp;
    struct addrinfo hints;
    char cport[5]={0};
    char dst[MAXSIZE];
    unsigned char message[MAXSIZE] = {0};
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
    if(use_udp == 0)
    {
        // Using AF_LOCAL caused "bind failed" 
        if ((sockfd = socket(address.sin_family, SOCK_STREAM, 0))<0)
        {
            // printf("Error with TCP socket creation\n");
            exit(1);
        }
    }
    else
    {
        if ((sockfd = socket(address.sin_family, SOCK_DGRAM, 0))<0)
        {
            // printf("Error with UDP socket creation\n");
            exit(1);
        }
    }

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(address.sin_family, dst, &address.sin_addr)<= 0) 
    {
        // printf("Address not supported_1\n");
        exit(0);
    }

    if(use_udp == 0)
    {
        if((sockfd_client = connect(sockfd, (struct sockaddr*) &address, sizeof(address))) < 0)
        {
            // printf("Connection failed\n");
            exit(1);
        }
    }
    // printf("Passed successfuly\n");
    while(1)
    {
        // if(sizeof(*message) == 256)
        // {
        //     size_t ret = fread(message, sizeof(*message), ARRAY_SIZE(message), fp);
        //     total = total + ret;
        // }
        // else
        // {
        //     size_t ret = fread(message, sizeof(*message), sizeof(*message), fp);
        //     total = total + ret;
        // }
        
        // size_t ret = fread(message, 1, 1, fp);
        size_t ret = fread(message, sizeof(*message), sizeof(*message), fp);
        total = total + ret;
        // if(ret != 1 )
        if(ret <= 0 )
        {
            // printf("Error! Ret value: %zu\n", ret);
            total = total + ret;
            // printf("Err! Ret value: %zu\n", total);
            if(use_udp==1)
            {
                sendto(sockfd, message, 0, MSG_CONFIRM, (const struct sockaddr*) &address, sizeof(address));
            }
            // if(send(sockfd, message, ret, 0) == -1)
            // {
            //     perror("Error in sending file\n");
            //     exit(1);
            // }
            exit(1);
        }
        if(use_udp == 0)
        {
            if(send(sockfd, message, ret, 0) == -1)
            {
                // perror("Error in sending file\n");
                exit(1);
            }
            bzero(message, MAXSIZE);
        }
        else
        {
            usleep(1);
            // if(send(sockfd, message, ret, 0) == -1)
            // {
            //     perror("Error in sending file\n");
            //     exit(1);
            // }
            if(sendto(sockfd, message, ret, MSG_CONFIRM, (const struct sockaddr*) &address, sizeof(address))  == -1)
            {
                // perror("Error in sending file\n");
                exit(1);
            }

            bzero(message, MAXSIZE);
        } 
        // printf("Ret value: %zu\n", total);
    }
    close(sockfd);
    exit(EXIT_SUCCESS);
}
