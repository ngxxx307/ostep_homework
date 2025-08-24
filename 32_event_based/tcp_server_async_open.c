#include <netinet/in.h> // sockaddr_in, INADDR_ANY
#include <sys/socket.h> // socket, bind, listen, accept, AF_INET, SOCK_STREAM
#include <sys/types.h>  // htonl, htons, socklen_t
#include <unistd.h>     // read, write, close
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <aio.h>
#include <stdbool.h>
#include <errno.h> // <-- ADD THIS LINE
#include <stdio.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include "aio_request.h"

#define PORT 8081
#define MAX_MESSAGE_SIZE 100
#define MAX_IO 100
#define DEBUG true

int server_socket;

aio_request *io_requests_list[MAX_IO];

void initServer(int server_socket, struct sockaddr_in *server_sockaddr_in)
{
    int flags = fcntl(server_socket, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl(F_GETFL)");
        exit(1);
    }
    if (fcntl(server_socket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl(F_SETFL)");
        exit(1);
    }

    if (bind(server_socket, (struct sockaddr *)server_sockaddr_in, sizeof(*server_sockaddr_in)) < 0)
    {
        perror("failed to bind socket.\n");
        exit(EXIT_FAILURE);
    }

    printf("Bind Socket Successfully.\n");

    if (listen(server_socket, 5) < 0)
    {
        perror("server failed to listen.\n");
        exit(EXIT_FAILURE);
    }
    printf("Listen to port: %d\n", PORT);
}

void newConnectionHandler(int server_socket, fd_set *mainReadFDs_ptr)
{
    struct sockaddr_in client_sockaddr_in;
    socklen_t len = sizeof(client_sockaddr_in);
    int conn;

    while (1)
    {
        printf("waiting for accept\n");
        conn = accept(server_socket, (struct sockaddr *)&client_sockaddr_in, &len);

        if (conn < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if (DEBUG)
                {
                    printf("No more pending connections.\n");
                }
                break; // Exit the loop
            }
            else
            {
                perror("accept");
                break; // Exit the loop
            }
        }

        // Add the new connection to the master set for select() to monitor.
        FD_SET(conn, mainReadFDs_ptr);
        printf("accepted\n");
        // Note: You would also need to update your `fd_max` variable for select() here.
    }
}

void timeHandler(int conn, fd_set *mainReadFDs_ptr)
{

    struct timeval tv;
    int result = gettimeofday(&tv, NULL);

    char res[50]; // Buffer to hold the formatted string
    sprintf(res, "The time now is: %ld\n", tv.tv_sec);
    write(conn, res, strlen(res));

    close(conn);

    FD_CLR(conn, mainReadFDs_ptr);
};

// Corrected openHandler
// This function now only INITIATES the aio_read for a file.
void openHandler(int conn, fd_set *mainReadFDs_ptr, char *token)
{
    char *file = strtok(NULL, " \n\r\t");
    if (DEBUG)
    {
        printf("token: %s.\n", token);
        printf("file: %s.\n", file);
    }
    if (file == NULL)
    {
        write(conn, "Error: Empty file name\n", 23);
        close(conn);
        FD_CLR(conn, mainReadFDs_ptr);
        return;
    }

    // open file
    int fd = open(file, O_RDONLY);
    if (fd < 0)
    {
        perror("Error opening file");
        write(conn, "Error: Could not open file.\n", 28);
        close(conn);
        FD_CLR(conn, mainReadFDs_ptr);
        return;
    }

    // Create an AIO request and associate the CLIENT's connection 'conn' with it.
    aio_request *req = create_aio_request(conn, "OPEN", fd);

    // Add the request to our list to be polled later
    for (int i = 0; i < MAX_IO; i++)
    {
        if (io_requests_list[i] == NULL)
        {
            io_requests_list[i] = req;
            // Start the asynchronous read
            printf("Starting aio_read for fd=%d, file='%s'\n", fd, file);
            int aio_result = aio_read(req->cb_ptr);
            printf("aio_read returned: %d\n", aio_result);
            if (aio_result < 0)
            {
                perror("aio_read failed to start");
                close(conn);
                FD_CLR(conn, mainReadFDs_ptr);
                free_aio_request(req);
                io_requests_list[i] = NULL; // Clean up
                return;
            }
            printf("AIO read for file '%s' started for client %d.\n", file, conn);
            // We DON'T close the connection here. We wait for the AIO to finish.
            // We also remove it from the select set so we don't read from it again.
            FD_CLR(conn, mainReadFDs_ptr);
            return;
        }
    }

    // If we get here, the AIO list is full
    write(conn, "Server busy, cannot process file read.\n", 39);
    close(fd);
    close(conn);
    FD_CLR(conn, mainReadFDs_ptr);
    free_aio_request(req);
}

// Corrected requestHandler
// This function is called when select() indicates the socket 'conn' is readable.
void requestHandler(int conn, fd_set *mainReadFDs_ptr)
{
    printf("Entering request handler...\n");
    char read_buffer[MAX_MESSAGE_SIZE] = {0}; // Initialize buffer to zeros

    // select() told us there is data, so we perform a synchronous read.
    ssize_t bytes_read = read(conn, read_buffer, sizeof(read_buffer) - 1);

    if (bytes_read <= 0)
    {
        // If read returns 0, the client has closed the connection.
        // If it's < 0, it's an error.
        if (bytes_read < 0)
        {
            perror("read");
        }
        printf("Client on fd %d disconnected.\n", conn);
        close(conn);
        FD_CLR(conn, mainReadFDs_ptr);
        return;
    }

    // Null-terminate the received data
    read_buffer[bytes_read] = '\0';

    char *token = strtok(read_buffer, " \n\r\t");

    if (DEBUG)
    {
        printf("buffer: %s.\n", read_buffer);
        printf("token: %s.\n", token);
    }

    if (token == NULL)
    {
        // Empty request, just close.
        close(conn);
        FD_CLR(conn, mainReadFDs_ptr);
        return;
    }

    if (strcmp(token, "time") == 0)
    {
        timeHandler(conn, mainReadFDs_ptr);
    }
    else if (strcmp(token, "open") == 0)
    {
        // Pass the connection descriptor 'conn' to the handler
        openHandler(conn, mainReadFDs_ptr, token);
    }
    else
    {
        write(conn, "Unknown command.\n", 17);
        close(conn);
        FD_CLR(conn, mainReadFDs_ptr);
    }
}

void sigint_handler(int sig)
{
    if (server_socket != -1)
    {
        close(server_socket);
        server_socket = -1;
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    printf("Caught signal %d (SIGINT). Exiting gracefully...\n", sig);
    exit(EXIT_SUCCESS);
}

// NEW function to check on the status of all pending AIO requests.
void checkAioCompletions()
{
    for (int i = 0; i < MAX_IO; i++)
    {
        if (io_requests_list[i] == NULL)
        {
            continue;
        }

        printf("Checking AIO request %d (client %d)...\n", i, io_requests_list[i]->conn);

        aio_request *req = io_requests_list[i];
        printf("Calling aio_error for request %d...\n", i);
        int status = aio_error(req->cb_ptr);
        printf("aio_error returned: %d (EINPROGRESS=%d)\n", status, EINPROGRESS);

        if (status == EINPROGRESS)
        {
            printf("Request %d still in progress, continuing...\n", i);
            // Still working, do nothing.
            continue;
        }

        printf("Request %d completed with status %d\n", i, status);
        int client_conn = req->conn;

        if (status == 0)
        {
            printf("AIO Success for client %d!\n", client_conn);
            ssize_t bytes_read = aio_return(req->cb_ptr);
            if (bytes_read > 0)
            {
                // Write the file content back to the client.
                write(client_conn, (const void *)req->cb_ptr->aio_buf, bytes_read);
            }
            else
            {
                write(client_conn, "File read successfully (0 bytes).\n", 34);
            }
        }
        else
        {
            printf("AIO failed for client %d!\n", client_conn);
            write(client_conn, "Error: AIO file read failed.\n", 29);
        }

        // Clean up
        close(req->cb_ptr->aio_fildes); // Close the file
        close(client_conn);             // Close the client connection
        free_aio_request(req);
        io_requests_list[i] = NULL;
    }
}

int main()
{
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        perror("Failed to register signal handler");
        return EXIT_FAILURE;
    }

    if (signal(SIGTERM, sigint_handler) == SIG_ERR)
    {
        perror("Failed to register signal handler");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_sockaddr_in;

    server_sockaddr_in.sin_port = htons(PORT);                   // port number
    server_sockaddr_in.sin_family = AF_INET;                     // IPV4
    server_sockaddr_in.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Listen to localhost only

    initServer(server_socket, &server_sockaddr_in);

    fd_set mainReadFDs, readFDs;
    FD_ZERO(&mainReadFDs);
    FD_SET(server_socket, &mainReadFDs); // Listen to server main port
    struct timeval timeout;

    while (1)
    {
        readFDs = mainReadFDs;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(FD_SETSIZE, &readFDs, NULL, NULL, &timeout) < 0)
        {
            perror("Select Error");
            continue;
        }

        // Loop over all file descriptor
        for (int fd = 0; fd < FD_SETSIZE; fd++)
        {
            if (!FD_ISSET(fd, &readFDs))
                continue;

            if (fd == server_socket)
            {
                newConnectionHandler(server_socket, &mainReadFDs);
            }
            else
            {
                requestHandler(fd, &mainReadFDs);
            }
        }

        checkAioCompletions();
    }
}