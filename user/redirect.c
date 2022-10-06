#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user.h"

int main()
{
    int pid = fork();

    if (pid == 0){
        close(1);
        open("output.txt",O_WRONLY | O_CREATE);

        // Now the file descriptor 1 represents "output.txt".

        char *argv[] = { "echo", "this", "is", "redirected", "echo", 0 };
        exec("echo", argv);
        printf("exec failed!\n");
        exit(1);
    } else {
        wait( (int*)0 );
    }
    exit(0);
}
