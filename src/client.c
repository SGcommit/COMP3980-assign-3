#include "../include/client.h"
#include <arpa/inet.h>
#include <sys/socket.h>

void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s -s <string> -t <conversion type: upper, lower, none>\n", prog_name);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int                opt;
    const char        *inputString    = NULL;
    const char        *conversionType = NULL;
    int                client_socket;
    struct sockaddr_in server_addr;
    ClientRequest      request;
    char               processed_string[STRING_SIZE];
    ssize_t            bytes_read;

    // Parse command-line arguments
    while((opt = getopt(argc, argv, "s:t:")) != -1)
    {
        switch(opt)
        {
            case 's':
                inputString = optarg;
                break;
            case 't':
                conversionType = optarg;
                break;
            default:
                print_usage(argv[0]);
        }
    }

    if(inputString == NULL || conversionType == NULL)
    {
        print_usage(argv[0]);
    }

    if(strcmp(conversionType, "upper") != 0 && strcmp(conversionType, "lower") != 0 && strcmp(conversionType, "none") != 0)
    {
        fprintf(stderr, "Invalid conversion type. Please choose from 'upper', 'lower', or 'none'.\n");
        exit(EXIT_FAILURE);
    }

    // Create and populate ClientRequest struct
    strncpy(request.string, inputString, sizeof(request.string) - 1);
    strncpy(request.conversionType, conversionType, sizeof(request.conversionType) - 1);

    // Create a TCP socket
    client_socket = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(client_socket == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORT_NUMBER);

    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if(connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Send request to server
    if(write(client_socket, &request, sizeof(ClientRequest)) == -1)
    {
        perror("Error writing to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Read processed string from server
    bytes_read = read(client_socket, processed_string, sizeof(processed_string) - 1);
    if(bytes_read == -1)
    {
        perror("Error reading from server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    processed_string[bytes_read] = '\0';
    printf("Processed string: %s\n", processed_string);

    close(client_socket);
    return 0;
}
