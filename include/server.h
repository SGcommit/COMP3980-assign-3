#ifndef SERVER_H
#define SERVER_H

#include <ctype.h>    // For toupper and tolower
#include <errno.h>
#include <fcntl.h>
#include <signal.h>    // For signal and SIGINT
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Constants for magic numbers
#define STRING_SIZE 256
#define CONVERSION_TYPE_SIZE 10
#define PORT_NUMBER 8080
#define BACKLOG_SIZE 10
#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif

// SET YOUR HANDLER HERE TO SHUT THE ERROR UP

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif

typedef struct
{
    char string[STRING_SIZE];
    char conversionType[CONVERSION_TYPE_SIZE];    // "upper", "lower", "none"
} ClientRequest;

// Function to process each client request (used in server)
noreturn void process_request(int client_socket);
void          custom_signal_handler(int signum);
#endif    // SERVER_H
