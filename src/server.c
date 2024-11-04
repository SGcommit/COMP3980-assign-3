#include "../include/server.h"


static volatile sig_atomic_t exit_flag = 0;    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// Signal handler to terminate the server
void custom_signal_handler(int signum)
{
    (void)signum;     // Mark parameter as unused
    exit_flag = 1;    // Set exit flag
}

// Function to process each client request
void process_request(int client_socket)
{
    ClientRequest request;
    char          result[STRING_SIZE] = {0};
    int           i;

    // Read the request from the client
    if(read(client_socket, &request, sizeof(ClientRequest)) <= 0)
    {
        perror("Error reading from client socket");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Copy the input string to result
    strncpy(result, request.string, sizeof(result) - 1);

    // Process the string based on conversionType
    if(strcmp(request.conversionType, "upper") == 0)
    {
        for(i = 0; result[i]; i++)
        {
            result[i] = (char)toupper(result[i]);
        }
    }
    else if(strcmp(request.conversionType, "lower") == 0)
    {
        for(i = 0; result[i]; i++)
        {
            result[i] = (char)tolower(result[i]);
        }
    }

    // Send the result back to the client
    if(write(client_socket, result, sizeof(result)) == -1)
    {
        perror("Error writing to client socket");
    }

    close(client_socket);
    exit(EXIT_SUCCESS);
}

int main(void)
{
    int                server_socket;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t          client_addr_len = sizeof(client_addr);

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif

    struct sigaction sa;
    sa.sa_handler = custom_signal_handler;    // Use the renamed signal handler
    sa.sa_flags   = 0;                        // No special flags
    sigemptyset(&sa.sa_mask);                 // No additional signals to block
#if defined(__clang__)
    #pragma clang diagnostic pop
#endif
    if(sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("Error setting up signal handler");
        return EXIT_FAILURE;
    }

    // Create a TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket == -1)
    {
        perror("Error creating socket");
        return EXIT_FAILURE;
    }

    // Configure server address
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT_NUMBER);

    if(bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Error binding socket");
        close(server_socket);
        return EXIT_FAILURE;
    }

    if(listen(server_socket, BACKLOG_SIZE) == -1)
    {
        perror("Error listening on socket");
        close(server_socket);
        return EXIT_FAILURE;
    }

    printf("Server is running...\n");

    while(!exit_flag)
    {
        int   client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        pid_t pid;
        if(client_socket == -1)
        {
            if(exit_flag)
            {
                break;    // Exit if the server is terminating
            }
            perror("Error accepting client connection");
            continue;
        }

        // Fork a new process to handle the request
        pid = fork();
        if(pid == -1)
        {
            perror("Error forking process");
            close(client_socket);
            continue;
        }
        if(pid == 0)
        {
            // In child process
            close(server_socket);              // Close the listening socket in child
            process_request(client_socket);    // Process the client request
        }
        else
        {
            // In parent process
            close(client_socket);    // Close the client socket in parent
        }
    }

    close(server_socket);
    printf("Server terminated gracefully.\n");
    return 0;
}
