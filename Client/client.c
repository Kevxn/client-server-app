#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <dirent.h>
#include "rdwrn.h"

#define INPUTSIZ 10

void get_student_id(int socket)
{
    int number_to_send = 1;
    int converted_number = htonl(number_to_send);

    write(socket, &converted_number, sizeof(converted_number));
    char sname_and_ip[64];
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));
    readn(socket, (unsigned char *) sname_and_ip, k);

    printf("%s\n", sname_and_ip);
}

void get_server_time(int socket)
{
    int number_to_send = 2;
    int converted_number = htonl(number_to_send);

    write(socket, &converted_number, sizeof(converted_number));

    char time_from_server[64];
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));
    readn(socket, (unsigned char *) time_from_server, k);

    printf("Server time: %s", time_from_server);
}

void get_uname_info(int socket)
{
    struct utsname unameData;

    printf("Third option chosen...\n");
    int number_to_send = 3;
    int converted_number = htonl(number_to_send);

    write(socket, &converted_number, sizeof(converted_number));

    size_t payload_length = sizeof(unameData);

    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));
    readn(socket, (unsigned char *) &unameData, payload_length);

    printf("Nodename:    %s\n", unameData.nodename);
    printf("Sysname:    %s\n", unameData.sysname);
    printf("Release:    %s\n", unameData.release);
    printf("Version:    %s\n", unameData.version);
    printf("Machine:    %s\n", unameData.machine);
}

void get_server_files(int socket){
    int number_to_send = 4;
    int converted_number = htonl(number_to_send);

    write(socket, &converted_number, sizeof(converted_number));

    char dir_string[256];
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));
    readn(socket, (unsigned char *) dir_string, k);

    printf("Directories in server folder:\n %s\n", dir_string);
}

void displaymenu()
{
    printf("0. Display menu\n");
    printf("1. Get student ID\n");
    printf("2. Get server time\n");
    printf("3. Get server uname data\n");
    printf("4. Get files in servers working directory\n");
    printf("5. Exit\n");
}

int main(void)
{
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Error - could not create socket");
	exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(50001);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)  {
	perror("Error - connect failed");
	exit(1);
    } else
       printf("Connected to server...\n");

    char input;
    char name[INPUTSIZ];

    displaymenu();

    do {
    printf("option> ");
    fgets(name, INPUTSIZ, stdin);
    name[strcspn(name, "\n")] = 0;
    input = name[0];
    if (strlen(name) > 1)
        input = 'x';
    switch (input) {
    case '0':
        displaymenu();
        break;
    case '1':
        get_student_id(sockfd);
        break;
    case '2':
        get_server_time(sockfd);
        break;
    case '3':
        get_uname_info(sockfd);
        break;
    case '4':
        get_server_files(sockfd);
        break;
    case '5':
        printf("Goodbye!\n");
        break;
    default:
        printf("Invalid choice - 0 displays options...!\n");
        break;
    }
    } while (input != '5');

    close(sockfd);

    exit(EXIT_SUCCESS);
}
