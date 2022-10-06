#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[]){
    
    /* Initialize */
    char data[17];
    data[0] = 3;
    for (int i = 1; i < 17; i++)
    {
        data[i] = data[i-1] + 2; 
    }
    printf("prime 2\n");
    int status = 0;
    
    /* Create pipe. */
    int p[2];
    pipe(p);
    /* Original process only writes. */
    write(p[1], data, 17);

    while (1)
    {
        /* Parent.*/
        if (fork() != 0){
            close(p[0]);
            close(p[1]);
            wait(&status);
            exit(0);
        }
        /* Child reads the data, cooks it, and writes it to the pipe.*/
        else
        {   
            int n = read(p[0], data, sizeof(data));
            char unit = data[0];
            printf("prime %d\n", unit, n);

            if (n == 1)        break;

            for (int i = 1; i < n; i++)
            {
                if (data[i] % unit != 0)
                {
                    write(p[1], &data[i], 1);
                }
            }            
        }
    }   
    exit(0);
}
