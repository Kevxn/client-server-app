// Cwk2: client.c - message length headers with variable sized payloads
//  also use of readn() and writen() implemented in separate code module

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
    // sending number to server so that correct method is fetched

    // part above works, below is for reading server response
    char sname_and_ip[64];
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));
    readn(socket, (unsigned char *) sname_and_ip, k);

    // printing student name and ID, concat'd with server IP
    printf("%s\n", sname_and_ip);
}

void get_server_time(int socket)
{
    int number_to_send = 2;
    int converted_number = htonl(number_to_send);

    write(socket, &converted_number, sizeof(converted_number));
    // sending number to server so that correct method is fetched

    // part above works for sending option to server, below is for displaying time
    char time_from_server[64];
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));
    readn(socket, (unsigned char *) time_from_server, k);
    // receiving data from the server

    printf("Server time: %s", time_from_server);
}

void get_uname_info(int socket)
{
    struct utsname unameData;
    // initializing utsname struct to get info about the servers machine
    // initializing in the client becuase it helps to know the size of the struct.

    printf("Third option chosen...\n");
    int number_to_send = 3;
    int converted_number = htonl(number_to_send);

    write(socket, &converted_number, sizeof(converted_number));
    // sending number to server so that correct method is fetched

    // code above selects the correct function from the server
    size_t payload_length = sizeof(unameData);
    // getting the size of the struct

    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));
    readn(socket, (unsigned char *) &unameData, payload_length);
    // reading struct from socket

    printf("Nodename:    %s\n", unameData.nodename);
    printf("Sysname:    %s\n", unameData.sysname);
    printf("Release:    %s\n", unameData.release);
    printf("Version:    %s\n", unameData.version);
    printf("Machine:    %s\n", unameData.machine);
    // printing information from struct
}

void get_server_files(int socket){
    int number_to_send = 4; 
    int converted_number = htonl(number_to_send);
    // serializing number to be sent over network using htonl function

    write(socket, &converted_number, sizeof(converted_number));
    // sending number to server so that correct method is fetched

    char dir_string[256]; // setting the dir_string buffer size
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));
    readn(socket, (unsigned char *) dir_string, k);

    // printing out the directories in the servers working dir
    // I delimited the different files by \n charachter for easy display.
    printf("Directories in server folder:\n %s\n", dir_string);
}

void displaymenu()
{
    // simply priting out menu
    printf("0. Display menu\n");
    printf("1. Get student ID\n");
    printf("2. Get server time\n");
    printf("3. Get server uname data\n");
    printf("4. Get files in servers working directory\n");
    printf("5. Exit\n");
}

int main(void)
{
    // *** this code down to the next "// ***" does not need to be changed except the port number
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Error - could not create socket");
	exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;

    // IP address and port of server we want to connect to
    serv_addr.sin_port = htons(50001);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // try to connect...
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)  {
	perror("Error - connect failed");
	exit(1);
    } else
       printf("Connected to server...\n");

    // ***
    // your own application code will go here and replace what is below... 
    char input;
    char name[INPUTSIZ];

    displaymenu();
    // display the menu..

    // start do while loop for the menu
    // close if the input is 5 -- exit
    do {
    printf("option> ");
    fgets(name, INPUTSIZ, stdin);   // get the value from input
    name[strcspn(name, "\n")] = 0;
    input = name[0];
    if (strlen(name) > 1)
        input = 'x';    // set invalid if input more 1 char
    switch (input) {
    case '0':
        displaymenu();
        break;
    case '1':
        // calling student ID method passing socket in
        get_student_id(sockfd);
        break;
    case '2':
        // calling server time method passing socket in
        get_server_time(sockfd);
        break;
    case '3':
        // calling uname info method passing socket in
        get_uname_info(sockfd);
        break;
    case '4':
        // calling server files display method passing socket in
        get_server_files(sockfd);
        break;
    case '5':
        // exit on 5
        printf("Goodbye!\n");
        break;
    default:
        // print error message if no above option is chosen
        printf("Invalid choice - 0 displays options...!\n");
        break;
    }
    // exit if choice is 5
    } while (input != '5');


    // cleaning up the sockets

    close(sockfd);

    exit(EXIT_SUCCESS);
} // end main()
