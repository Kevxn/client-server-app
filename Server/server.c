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

void *client_handler(void *);
void sigint_handler();

void send_hello(int);
void send_student_id(int);
void send_local_time(int);
void send_uname_info(int);
void send_server_files(int);
int listen_user_choice(int);

struct timeval tv1, tv2;

int main(void)
{
    if (gettimeofday(&tv1, NULL) == -1){
        perror("gettimeofday error");
        exit(EXIT_FAILURE);
    }

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

    puts("Waiting for incoming connections...");
    while (1) {
	printf("Waiting for a client to connect...\n");
	connfd =
	    accept(listenfd, (struct sockaddr *) &client_addr, &socksize);
	printf("Connection accepted...\n");

	pthread_t sniffer_thread;

	if (pthread_create
	    (&sniffer_thread, NULL, client_handler,
	     (void *) &connfd) < 0) {
	    perror("could not create thread");
	    exit(EXIT_FAILURE);
	}

	printf("Handler assigned\n");
    }

    exit(EXIT_SUCCESS);
}

void *client_handler(void *socket_desc)
{
    int connfd = *(int *) socket_desc;
    int user_choice = 0;

    do{

       user_choice = listen_user_choice(connfd);

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

    return 0;
}

void sigint_handler(){

    if (gettimeofday(&tv2, NULL) == -1){
        perror("gettimeofday error");
        exit(EXIT_FAILURE);
    }

    printf("\nCaptured <ctrl + c>\n");
    printf("Server runtime: %f seconds\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));

    exit(EXIT_SUCCESS);
    
}

int listen_user_choice(int socket){
    int menu_choice = 0;

    read(socket, &menu_choice, sizeof(menu_choice));
    return ntohl(menu_choice);
}

void send_local_time(int socket){
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char *time_string = asctime(timeinfo);

    size_t n = strlen(time_string) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));
    writen(socket, (unsigned char *) time_string, n);
}

void send_student_id(int socket1){
    char sname[26] = "---Kevin | S1715611";
    char ipaddr[16];

    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    strcpy(ipaddr, inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));

    strcat(ipaddr, sname);

    size_t n = strlen(ipaddr) + 1;
    writen(socket1,(unsigned char *) &n, sizeof(size_t));
    writen(socket1, (unsigned char *) ipaddr, n);
}

void send_uname_info(int socket){
    
    struct utsname unameData;
    uname(&unameData);

    size_t payload_length = sizeof(unameData);
    printf("payload length: %zu\n", payload_length);

    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));
    writen(socket, (unsigned char *) &unameData, payload_length);
}

void send_server_files(int socket){
    char dir_string[256];

    DIR *mydir;
    if ((mydir = opendir(".")) == NULL) {
    perror("error");
    exit(EXIT_FAILURE);
    }

    struct dirent *entry = NULL;

    while ((entry = readdir(mydir)) != NULL){
        strcat(dir_string, entry->d_name);
        strcat(dir_string, "\n");
    }

    size_t payload_length = sizeof(dir_string);

    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));
    writen(socket, (unsigned char *) dir_string, payload_length);
}

