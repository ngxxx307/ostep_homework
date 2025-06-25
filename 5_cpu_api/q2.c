#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Open a file for writing. Create it if it doesn't exist,
    // and truncate it to zero length if it does.
    // Permissions are set to read/write for the owner.
    int fd = open("concurrent_output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd == -1) {
        perror("open");
        exit(1);
    }

    // Fork the current process to create a child process.
    pid_t rc = fork();

    if (rc < 0) {
        // Fork failed; print an error and exit.
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        // This block is executed by the child process.
        const char *child_msg = "This is the CHILD writing.\n";
        for (int i = 0; i < 10; i++) {
            // The child writes its message to the file.
            // The write() call is atomic, so this string will not be broken up.
            // However, the scheduler might interrupt the child after this write
            // and let the parent run.
            write(fd, child_msg, strlen(child_msg));
        }
        printf("Child process finished writing.\n");
    } else {
        // This block is executed by the parent process.
        const char *parent_msg = "This is the PARENT writing.\n";
        for (int i = 0; i < 10; i++) {
            // The parent writes its message to the file.
            // Because the file offset is shared with the child, the parent's
            // writes will continue from wherever the child left off, and vice-versa.
            write(fd, parent_msg, strlen(parent_msg));
        }
        
        // The parent waits for the child process to terminate before continuing.
        // This ensures the child finishes its work before the parent closes the file.
        wait(NULL);
        printf("Parent process finished writing and child has terminated.\n");
    }

    close(fd);

    return 0;
}