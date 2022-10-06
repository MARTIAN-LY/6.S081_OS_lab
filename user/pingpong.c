#include "kernel/types.h"
#include "user/user.h"


int main(int argc, char* argv[]){

    /* 
        p[0]: read end
        p[1]: write end
     */
    int p1[2];
    int p2[2];
    pipe(p1);
    pipe(p2);

    if ( fork() == 0 ){
        /* child */
        close(p1[1]);
        close(p2[0]);

        /* read from p1*/
        char msg_get[5];
        read(p1[0],msg_get,4);
        close(p1[0]);
        printf("%d: received %s\n",getpid(), msg_get);

        /* write to p2*/
        write(p2[1],"pong",4);
        close(p2[1]);

    } else {
        /* parent */
        close(p1[0]);
        close(p2[1]);
        
        /* write to p1*/
        write(p1[1],"ping",4);
        close(p1[1]);

        /* read from p2*/
        char* msg_get[5];
        read(p2[0],msg_get,4);
        close(p2[0]);
        printf("%d: received %s\n",getpid(), msg_get);
    }
    
    exit(0);
}