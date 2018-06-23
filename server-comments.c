// Cwk2: server.c - multi-threaded server using readn() and writen()

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "rdwrn.h"

// thread function
void *client_handler(void *);
void sigint_handler();

void send_hello(int);
void send_student_id(int);
void send_local_time(int);
void send_uname_info(int);
void send_server_files(int);
int listen_user_choice(int);

struct timeval tv1, tv2; // for calculating execution time
// you shouldn't need to change main() in the server except the port number
int main(void)
{
    if (gettimeofday(&tv1, NULL) == -1){
        perror("gettimeofday error");
        exit(EXIT_FAILURE);
    }

    // setting up the signal handler for capturing SIGINT
    // singal function calls SIGINT handler method where execution time
    // is calculated
    signal(SIGINT, sigint_handler);
    

    int listenfd = 0, connfd = 0;

    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t socksize = sizeof(struct sockaddr_in);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(50001);

    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (listen(listenfd, 10) == -1) {
	perror("Failed to listen");
	exit(EXIT_FAILURE);
    }
    // end socket setup

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    while (1) {
	printf("Waiting for a client to connect...\n");
	connfd =
	    accept(listenfd, (struct sockaddr *) &client_addr, &socksize);
	printf("Connection accepted...\n");

	pthread_t sniffer_thread;

        // third parameter is a pointer to the thread function, fourth is its actual parameter
	if (pthread_create
	    (&sniffer_thread, NULL, client_handler,
	     (void *) &connfd) < 0) {
	    perror("could not create thread");
	    exit(EXIT_FAILURE);
	}
	//Now join the thread , so that we dont terminate before the thread
	//pthread_join( sniffer_thread , NULL);
	printf("Handler assigned\n");
    }

    // never reached...
    // ** should include a signal handler to clean up
    exit(EXIT_SUCCESS);
} // end main()

void *client_handler(void *socket_desc)
{
    int connfd = *(int *) socket_desc;
    int user_choice = 0;

    do{

       user_choice = listen_user_choice(connfd);
       // while the user is connected, listen for their input

       if (user_choice == 1){
            printf("Call 1st method\n");
            send_student_id(connfd);
       }
       else if (user_choice == 2){
            printf("Call 2nd method\n");
            send_local_time(connfd);
       }
       else if(user_choice == 3){
            printf("Call 3rd method\n");
            send_uname_info(connfd);
       }
       else if(user_choice == 4){
        printf("Call 4th method\n");
        send_server_files(connfd);
       }
    } while(user_choice != 0);

    shutdown(connfd, SHUT_RDWR);
    close(connfd);

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());
    // print thread exiting info

    return 0;
}

void sigint_handler(){
    // this function is for when SIGINT is captured
    // this method is called and appropriate message is displayed
    // it also calculates the execution time of the server

    if (gettimeofday(&tv2, NULL) == -1){
        perror("gettimeofday error");
        exit(EXIT_FAILURE);
        // display approproate error message
    }

    printf("\nCaptured <ctrl + c>\n");
    // display capture message
    printf("Server runtime: %f seconds\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));
    // calculate runtime

    // exit gracefully
    exit(EXIT_SUCCESS);
}

int listen_user_choice(int socket){
    // This method is for listening to the connected clients input
    // accepts a socket as a parameter
    int menu_choice = 0;

    read(socket, &menu_choice, sizeof(menu_choice));
    // read input from socket

    // return and convert network serialized number back to int
    return ntohl(menu_choice);
}

void send_local_time(int socket){
    // sends the local time of the server to the client
    // uses time struct
    // accepts a socket as param
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char *time_string = asctime(timeinfo);
    // using asctime method to convert to string

    size_t n = strlen(time_string) + 1;
    // + 1 on string for null terminator
    // send string through socket to client
    writen(socket, (unsigned char *) &n, sizeof(size_t));
    writen(socket, (unsigned char *) time_string, n);
}

void send_student_id(int socket1){
    // sends the student id and concat's ip address to the beginning
    // accepts a socket as a param

    // hardcoded student name and ID
    char sname[26] = "---Kevin Gray | S1715611";
    // IP address will be determined by code below
    char ipaddr[16];

    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    strcpy(ipaddr, inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));
    // getting IP address..

    // concatenating name onto IP address so that student info is
    // prefixed by the IP as required
    strcat(ipaddr, sname);

    size_t n = strlen(ipaddr) + 1;
    // + 1 for null terminator

    // sending data down through socket
    writen(socket1,(unsigned char *) &n, sizeof(size_t));
    writen(socket1, (unsigned char *) ipaddr, n);
}

void send_uname_info(int socket){
    // sends the uname information to the client
    // accepts a socket as param
    // initialize utsname struct

    struct utsname unameData;
    uname(&unameData);

    size_t payload_length = sizeof(unameData);
    // get the size of the struct
    printf("payload length: %zu\n", payload_length);

    // write header and actual struct to the socket
    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));
    writen(socket, (unsigned char *) &unameData, payload_length);
}

void send_server_files(int socket){
    // uses dirent.h header to get information about directories
    // sends the information about folders/files in working directory
    // to the client
    // accepts socket as param
    char dir_string[256];

    // init pointer to directory
    DIR *mydir;
    // open the current (.) directory
    if ((mydir = opendir(".")) == NULL) {
    perror("error");
    // display appropriate error on failure
    exit(EXIT_FAILURE);
    }

    struct dirent *entry = NULL;
    // using dirent header file

    while ((entry = readdir(mydir)) != NULL){
        // while we can read the directory
        // concat the filename to the large fir_string buffer
        strcat(dir_string, entry->d_name);
        // and the concat a newline to the end, meaning that display is 
        // a lot easier and I dont have to fish through the string for a
        // delimiter on the client
        strcat(dir_string, "\n");
    }

    // getting the size to send the header
    size_t payload_length = sizeof(dir_string);

    // writing the header to the socket
    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));
    // writing the actual string to the socket
    writen(socket, (unsigned char *) dir_string, payload_length);
}

