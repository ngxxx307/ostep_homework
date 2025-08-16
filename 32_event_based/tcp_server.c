#include <stdio.h>      // printf
#include <netinet/in.h> // sockaddr_in, INADDR_ANY
#include <sys/socket.h> // socket, bind, listen, accept, AF_INET, SOCK_STREAM
#include <sys/types.h>  // htonl, htons, socklen_t
#include <unistd.h>     // read, write, close
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#define PORT 8080
#define MAX_MESSAGE_SIZE 100

int main()
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_sockaddr_in; // Changed from pointer to struct

    server_sockaddr_in.sin_port = htons(PORT);                   // port number
    server_sockaddr_in.sin_family = AF_INET;                     // IPV4
    server_sockaddr_in.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // Listen to localhost only

    if (bind(server_socket, (struct sockaddr *)&server_sockaddr_in, sizeof(server_sockaddr_in)) < 0)
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

    // Client

    struct sockaddr_in client_sockaddr_in;
    socklen_t len = sizeof(client_sockaddr_in);
    char read_buffer[MAX_MESSAGE_SIZE] = {};
    int conn;
    struct timeval tv;

    while (1)
    {
        conn = accept(server_socket, (struct sockaddr *)&client_sockaddr_in, &len);

        read(conn, read_buffer, sizeof(read_buffer));

        int result = gettimeofday(&tv, NULL);

        char res[50]; // Buffer to hold the formatted string
        sprintf(res, "The time now is: %ld\n", tv.tv_sec);
        write(conn, res, strlen(res));

        close(conn);
    }
}