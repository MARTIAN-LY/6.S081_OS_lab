#include "kernel/types.h"
#include "user.h"

int main()
{
    int pid, status;

    pid = fork();
    if (pid == 0)
    {
        char* argv[] = {"echo","THIS","IS","ECHO",0};
        exec("echo",argv);
        printf("exec failed!");
        exit(1);
    } else {
        printf("parent waiting\n");
        wait(&status);
        printf("the child exited with status %d\n",status);
    }

    exit(0);
}
