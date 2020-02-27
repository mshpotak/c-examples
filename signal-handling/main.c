// by Mykhailo SHPOTAK, Ivan PANCHENKO
// https://github.com/mshpotak/c-examples/signal-handling

//signal management
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
//file control
#include <fcntl.h>
//standard input/output functions
#include <stdio.h>
//for explicit exit()
#include <stdlib.h>
//for perror() and errno
#include <errno.h>
//for time management
#include <time.h>
//string management
#include <string.h>

#define BUFFER_SIZE 256
#define FILE2MODE 0644

//decalre global log file descriptor
int fd_log;

//function to handle signals upon termination
void signal_handler(int signum, siginfo_t *si, void *ucontext){
    int result;
    char buffer[BUFFER_SIZE];
    time_t curtime;
    time(&curtime);

    //make a log about about input signal
    result = sprintf(buffer, "%.24s: Program terminated\nTermination info: \n\tSIGNO: %d\n\tSIGERR: %d\n\tSIGCD: %d\n", ctime(&curtime), si->si_signo, si->si_errno, si->si_code);
    if( result == -1 ){
        perror("sprintf error:");
        close(fd_log);
        exit(-1);
    }
    result = write(fd_log, buffer, strlen(buffer));
    if( result == -1 ){
        perror("write error:");
        close(fd_log);
        exit(-1);
    }

    exit(0);
}

int main(int argc, const char *argv[]){

    //open a log file
    fd_log = open("log.txt", O_CREAT|O_TRUNC|O_RDWR, FILE2MODE);
    if(fd_log == -1){
        perror("fd_log open error:");
        return 0;
    }

    int result;
    char buffer[BUFFER_SIZE];
    pid_t pid = getpid();
    time_t curtime;
    time(&curtime);

    //make a log entry
    result = sprintf(buffer, "%.24s: Program started with PID: %d\n", ctime(&curtime), pid);
    if(result == -1){
        perror("main sprintf error:");
        close(fd_log);
        return 0;
    }
    result = write(fd_log, buffer, strlen(buffer));
    if( result == -1 ){
        perror("main write error:");
        close(fd_log);
        exit(-1);
    }

    //initialize sigaction structure that points to the custom signal_handler() function
    const struct sigaction action = {
        // void (*sa_handler)(int);
        // void (*sa_sigaction)(int, siginfo_t *, void *);
        .sa_sigaction = &signal_handler,
        // sigset_t sa_mask;
        // int sa_flags;
        .sa_flags = SA_SIGINFO
        // void (*sa_restorer)(void);
    };
    struct sigaction action_old;

    // set an action for SIGHUP signal
    // int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
    sigaction(SIGHUP, &action, &action_old);

    //infinite loop that makes periodical logs
    while(1){
        sleep(2);
        time(&curtime);
        result = sprintf(buffer, "%.24s: Waiting\n", ctime(&curtime));
        if(result == -1){
            perror("main sprintf error:");
            close(fd_log);
            return 0;
        }

        result = write(fd_log, buffer, strlen(buffer));
        if( result == -1 ){
            perror("main write error:");
            close(fd_log);
            exit(-1);
        }
    }

    return 0;
}
