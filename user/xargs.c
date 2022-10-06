#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAXLEN 1024

int readargs(char *strs[], int offset)
{
    char line[MAXLEN];
    int num_of_args = 0;
    int end = 0;
    int current = 0;

    // Read till '\n' appears.
    while (read(0, line + current, 1))
    {
        if (current == MAXLEN)
        {
            printf("xargs: too long arguments.\n");
            exit(2);
        }
        // Replace ' ' with 0
        if (line[current] == ' ')
        {
            line[current] = 0;
        }
        // Replace the last '\n' with 0
        if (line[current] == '\n')
        {
            line[current] = 0;
            end = current + 1;
            break;
        }
        current++;
    }
    // Split the line.
    for (int i = 0; i < end; i++)
    {
        if (line[i] == 0)
        {
            continue;
        }
        if (offset >= MAXARG)
        {
            printf("xargs: too many arguments.\n");
            exit(1);
        }
        char* p = line + i;
        int len = strlen(p);
        strs[offset] = (char *)malloc(len + 1);
        strcpy(strs[offset], p);
        offset++;
        num_of_args++;
        i += len;
    }

    // Mark the end of args.
    for (int i = offset; i < MAXARG + 1; i++)
    {
        strs[offset] = (char*)malloc(1);
        strs[offset][0] = 0;
    }
    return num_of_args;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("xargs: needs more arguments.\n");
        exit(1);
    }
    if (argc > MAXARG)
    {
        printf("xargs: too many arguments.\n");
        exit(1);
    }

    // args[][] will be used in the new process.
    char *args[MAXARG + 1];
    for (int i = 1; i < argc; i++)
    {
        args[i - 1] = (char *)malloc(sizeof argv[i]);
        strcpy(args[i - 1], argv[i]);
    }
    int status;
    
    while (readargs(args, argc-1) != 0)
    {
        if (fork() != 0)
        {
            wait(&status);
        } else {
            exec(args[0],args);
            printf("xargs: exec failed.\n");
            exit(3);
        }
    }
    
    exit(0);
}