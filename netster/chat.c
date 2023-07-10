#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <pthread.h>

#define MAXSIZE 256
#define MAX_CLIENT 5
// long port;  
int use_udps, sockfd_s;
// char* iface;
int client_sockets[MAX_CLIENT], number_of_clients=0;
char message[MAXSIZE]={0}, server_msg[MAXSIZE]={0};
// struct sockaddr_in client_address;
/*
 *  Here is the starting point for your netster part.1 definitions. Add the 
 *  appropriate comment header as defined in the code formatting guidelines
 *
 *   Referring: 
 *    1. https://www.geeksforgeeks.org/socket-programming-cc/
 *    2. https://dev.to/sanjayrv/a-beginners-guide-to-socket-programming-in-c-5an5
 *    3. https://www.geeksforgeeks.org/udp-server-client-implementation-c/
 *    4. https://stackoverflow.com/questions/21405204/multithread-server-client-implementation-in-c
 *    5. Collaborated with a current CN student 'nvaradha'
 *
 *
 * Add function definitions
 * iface=<ip_address>, port=8080, for TCP (use_udp=0), UDP(use_udp=1)
 * TCP: ./netster -p 8081 -i 127.0.0.1 
 * UDP: ./netster -p 8081 -i 127.0.0.1 -u
 */

struct thread_tcp_arg
{
    int new_sockets; char ifaces[100]; int ports, hosts;
};

void *thread_connections(void* args)
{
    char client_message[MAXSIZE]; 
    int valread;
    struct thread_tcp_arg *arg = (struct thread_tcp_arg*) args;
    struct thread_tcp_arg arg1 = *arg;
    int new_socket = arg1.new_sockets;
    char iface[100];
    int slen = strlen(arg1.ifaces);
    arg1.ifaces[slen]='\0';
    strncpy(iface, arg1.ifaces, slen+1);
    int port = arg1.ports;
    // int host = arg1.hosts;
    

    while(1)
    {
         
        bzero(client_message, sizeof(client_message));
        valread = read(new_socket, client_message, MAXSIZE);
        if(valread > 0)
        {
            
            if(strcmp(client_message, "hello\n")==0)
            {
                printf("got message from ('%s', %d)\n", iface, port);
                send(new_socket, "world\n", strlen("world\n"), 0);
            }
            else if(strcmp(client_message, "goodbye\n")==0)
            {
                printf("got message from ('%s', %d)\n", iface, port);
                send(new_socket, "farewell\n", strlen("farewell\n"), 0);
                close(new_socket);
                pthread_exit(NULL);
            }
            else if(strcmp(client_message, "exit\n")==0)
            {
                printf("got message from ('%s', %d)\n", iface, port);
                send(new_socket, "ok\n", strlen("ok\n"), 0);
                for(int i=0;i<number_of_clients;i++)
                {
                    if(new_socket !=client_sockets[i])
                    {
                        send(client_sockets[i], "ok\n", strlen("ok\n"), 0);
                        close(client_sockets[i]);
                    }
                    
                }
                close(new_socket);
                shutdown(sockfd_s, SHUT_RDWR);
                exit(3);
            }
            else
            {
                printf("got message from ('%s', %d)\n", iface, port);
                send(new_socket, client_message, strlen(client_message), 0);
            }
        }
    }
    
} 

void UDP_server(int sockfd, int iface, int port)
{
    char client_message[MAXSIZE]; 
    bzero(client_message, sizeof(client_message));
    struct sockaddr_in client_udp_address;
    bzero(&client_udp_address, sizeof(client_udp_address));
    unsigned int client_length = sizeof(client_udp_address); 
    int readval;
    char cli_addr[50];
    readval = recvfrom(sockfd, (char *) client_message, MAXSIZE, MSG_WAITALL, (struct sockaddr*) &client_udp_address, &client_length);
    client_message[readval]='\0';
    void *raw_addr;
    struct sockaddr_in* tmp = (struct sockaddr_in*)&client_udp_address;
    raw_addr = &(tmp->sin_addr);
    inet_ntop(client_udp_address.sin_family, raw_addr, cli_addr, 50);
    if(readval > 0)
    {
        if(strcmp(client_message, "hello\n")==0)
        {
            printf("got message from ('%s', %d)\n", cli_addr, client_udp_address.sin_port);
            sendto(sockfd,(const char*) "world\n", strlen("world\n"), MSG_CONFIRM, (const struct sockaddr*) &client_udp_address, client_length);
        }
        else if(strcmp(client_message, "goodbye\n")==0)
        {
            printf("got message from ('%s', %d)\n", cli_addr, client_udp_address.sin_port);
            sendto(sockfd,(const char*) "farewell\n", strlen("farewell\n"), MSG_CONFIRM, (const struct sockaddr*) &client_udp_address, client_length);
        }
        else if(strcmp(client_message, "exit\n")==0)
        {
            printf("got message from ('%s', %d)\n", cli_addr, client_udp_address.sin_port);
            sendto(sockfd,(const char*) "ok\n", strlen("ok\n"), MSG_CONFIRM, (const struct sockaddr*) &client_udp_address, client_length);
            close(sockfd);
            shutdown(sockfd, SHUT_RDWR);
            // sleep(5);
            exit(0);
        }
        else
        {
        
            printf("got message from ('%s', %d)\n", cli_addr, client_udp_address.sin_port);
            sendto(sockfd,(const char*) client_message, strlen(client_message), MSG_CONFIRM, (const struct sockaddr*) &client_udp_address, client_length);

        }
    }
}

void chat_server(char* iface, long port, int use_udp) 
{
    port = port; use_udps=use_udp; iface=iface;
    pthread_t threading;
    char cport[5]={0};
    sprintf(cport, "%ld", port);
    struct addrinfo *result, *rp;
    char cli_addr[50];

    
    // creation of socket for the localhost, change AF_LOCAL instead.
    struct sockaddr_in address;
    // struct sockaddr client_udp_address;
    bzero(&address, sizeof(address));
    // bzero(&client_udp_address, sizeof(client_udp_address));
    int sockfd, new_socket=0, opt=1, addrlen = sizeof(address);
    // char client_message[MAXSIZE];
    struct addrinfo hints;
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
            // printf("Error with TCP socket creation\n");
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
    sockfd_s = sockfd;
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
                void *raw_addr;
                struct sockaddr_in* tmp = (struct sockaddr_in*)&address;
                raw_addr = &(tmp->sin_addr);
                inet_ntop(address.sin_family, raw_addr, cli_addr, 50);
                printf("connection %d from ('%s', %d)\n", number_of_clients, cli_addr, address.sin_port);
                client_sockets[number_of_clients] = new_socket;
                number_of_clients=number_of_clients+1;
            }  
            struct thread_tcp_arg a;
            // int slen = strlen(iface);
            // iface[slen]='\0';
            // strncpy(a.ifaces, iface, slen+1);
            a.new_sockets = new_socket;
            // a.ports = port; 
            a.ports = address.sin_port;
            
            a.hosts = address.sin_addr.s_addr;
            int slen = strlen(cli_addr);
            cli_addr[slen]='\0';
            strncpy(a.ifaces, cli_addr, slen+1);
        
            if((pthread_create(&threading, NULL, thread_connections, (void*)&a)) < 0)
            {
                // printf("Error in thread creation process\n");
                exit(1);
            }  
        } //End of TCP Server
        else // UDP Server
        {
            while(1)
            {
                UDP_server(sockfd, address.sin_addr.s_addr, address.sin_port);
            }
        } 
    }        
}

struct thread_client_arg
{
    int sockfd, use_udp; struct sockaddr_in address; char message[MAXSIZE];
};

void* client_thread(void* args)
{
    // int* sockfd1 = (int*) sockfdd;
    // int sockfd = *sockfd1;
    int valread=0;
    struct thread_client_arg *arg = (struct thread_client_arg*) args;
    struct thread_client_arg arg1 = *arg;
    struct sockaddr_in client_address = arg1.address;
    int sockfd = arg1.sockfd;
    int use_udp = arg1.use_udp;
    int slen=strlen(arg1.message);
    arg1.message[slen]='\0';
    char message[MAXSIZE];
    strncpy(message, arg1.message, slen+1);
    unsigned int server_length = sizeof(client_address);

    while(1)
    {
        bzero(server_msg, MAXSIZE);
        
        if(use_udp==1)
        {
            valread=recvfrom(sockfd, (char *)server_msg, MAXSIZE, 0, (struct sockaddr*) &client_address, &server_length);
            server_msg[valread]='\0';
        }
        else
        {
            valread=recv(sockfd, server_msg, MAXSIZE, 0);
            // server_msg[valread]='\0';
        }
        if(valread>0)
        {
            printf("%s", server_msg);
            if((strcmp(server_msg, "farewell\n")==0 && strcmp(message, "farewell\n")!=0) || (strcmp(server_msg, "ok\n")==0 && strcmp(message, "ok\n")!=0))
            {
                close(sockfd);
                exit(0);
                break;
            }
        }
    }
    return NULL;
}   

// ./netster -p 8081 127.0.0.1
void chat_client(char* host, long port, int use_udp) 
{
    int sockfd=0, sockfd_client;
    struct sockaddr_in address;
    char cport[5]={0};
    int valread=0;
    sprintf(cport, "%ld", port);

    // Figuring out the IP
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;
    struct addrinfo *result, *rp;

    // Source: getaddrinfo man page
    int s = getaddrinfo(host, cport, &hints, &result);
    if(s != 0)
    {
        // fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }
    char dst[MAXSIZE];
    void* raw_addr;

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
    struct sockaddr_in client_address = address;
    unsigned int server_length = sizeof(client_address);
    while(1)
    {

        bzero(message, MAXSIZE);
        bzero(server_msg, MAXSIZE);

        char* dummy = fgets(message, MAXSIZE, stdin);
        if(dummy==NULL)
        {
            exit(1);
        }

        if(use_udp == 0)
        {
            send(sockfd, message, strlen(message), 0);
            valread=recv(sockfd, server_msg, MAXSIZE, 0);
        }
        else
        {
            sendto(sockfd, (const char*) message, strlen(message), MSG_CONFIRM, (const struct sockaddr*) &address, sizeof(address));
            valread=recvfrom(sockfd, (char *)server_msg, MAXSIZE, 0, (struct sockaddr*) &client_address, &server_length);
            server_msg[valread]='\0';
        }   

        if(valread>0)
        {
            printf("%s", server_msg);
            if((strcmp(server_msg, "farewell\n")==0 && strcmp(message, "farewell\n")!=0) || (strcmp(server_msg, "ok\n")==0 && strcmp(message, "ok\n")!=0))
            {
                close(sockfd);
                exit(0);
                break;
            }
        }
        
    }
    
}

