// by Mykhailo SHPOTAK
// https://github.com/mshpotak/c-examples/process-daemonization

//process management
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <signal.h>
//file control
#include <fcntl.h>
//standard input/output functions
#include <stdio.h>
//for perror() and errno
#include <errno.h>
//for explicit exit()
#include <stdlib.h>
//for time management
#include <time.h>
//string management
#include <string.h>

#define FILE2MODE 0644

//log info input function
int log_entry(int file_fd, const char* note){
    int result;
    time_t log_timestamp;
    char log_info[256];
    time( &log_timestamp );

    sprintf( log_info, "PID# %d; %.24s; \n%s\n\n", getpid(), ctime( &log_timestamp ), note );

    result = write( file_fd, (const char*)log_info, strlen(log_info) );
    if( result == -1 ){
        perror("write error:");
        if(close(file_fd) == -1){
            perror("close error:");
        };
        return -1;
    }

    //printf( "New log entry:\n \t%s\n", log_info );
    return 0;
}

int main(){
    //create a file for logging process info
    int file_fd;
    file_fd = open( "log.txt", O_CREAT|O_RDWR|O_TRUNC, FILE2MODE );
    if( file_fd == -1 ){
        perror("open error:");
        return 0;
    }

    int result;
    if( log_entry(file_fd, "log started") == -1){
        return 0;
    }
    //get main process parameters
    pid_t pid_main = getpid();
    pid_t ppid_main = getppid();
    pid_t sid_main = getsid(pid_main);
    pid_t tid_main = syscall(SYS_gettid);
    uid_t uid_main = getuid();
    gid_t gid_main = getgid();

    printf("Main process IDs:\n");
    printf("PID: %d\n", pid_main);
    printf("PPID: %d\n", ppid_main);
    printf("SID: %d\n", sid_main);
    printf("TID: %d\n", tid_main);
    printf("UID: %d\n", uid_main);
    printf("GID: %d\n\n", gid_main);

    //declare variables for process management
    pid_t pid_daemon;
    pid_t pid_current;

    //fork a child process and initialize variables for process management
    result = fork();
    if( result == -1 ){
        perror("fork error:");
        if( close(file_fd) == -1 ){
            perror("close error:");
        };
        return 0;
    } else if ( result == 0){
        pid_daemon = getpid();
    } else {
        pid_daemon = result;
    }
    pid_current = getpid();

    //main process exlusive if statement
    if(pid_current == pid_main){/
        //make a log entry
        if( log_entry(file_fd, "fork process started") == -1 ){
            if( close(file_fd) == -1 ){
                perror("close error:");
            };
            kill(pid_daemon, SIGKILL);
            return 0;
        }
        //make a log entry
        if( log_entry(file_fd, "main process closed") == -1 ){
            if(close(file_fd) == -1){
                perror("close error:");
            };
            kill(pid_daemon, SIGKILL);
            return 0;
        }
        //exit the main process
        exit(0);
    }

    //now a child process becomes session leader and a parent process
    //set session identifier for a 1st child process
    pid_t sid_daemon = setsid();
    if( sid_daemon == -1 ){
        perror("setsid error:");
    }

    //fork a child process (future daemon) and set its process identifier
    pid_main = pid_daemon;
    result = fork();
    if( result == -1 ){
        perror("fork error:");
        if( close(file_fd) == -1 ){
            perror("close error:");
        };
        return 0;
    } else if ( result == 0){
        pid_daemon = getpid();
    } else {
        pid_daemon = result;
    }
    pid_current = getpid();


    if(pid_current == pid_main){
        //make a log entry
        if( log_entry(file_fd, "daemon process started") == -1 ){
            if( close(file_fd) == -1 ){
                perror("close error:");
            };
            kill(pid_daemon, SIGKILL);
            return 0;
        }
        //make a log entry
        if( log_entry(file_fd, "fork process closed") == -1 ){
            if(close(file_fd) == -1){
                perror("close error:");
            };
            kill(pid_daemon, SIGKILL);
            return 0;
        }
        //exit the current parent process
        exit(0);
    }

    //change a directory to root directory
    if( chdir("/") == -1 ){
        perror("chdir error:");
        if( close(file_fd) == -1 ){
            perror("close error:");
        };
        return 0;
    }

    // int stdin_copy = dup(0);
    // int stdout_copy = dup(1);
    // int stderr_copy = dup(2);

    //close standard io streams
    if(close(file_fd) == -1){
        perror("close file_fd error:");
        return 0;
    };
    if(close(0) == -1){
        perror("close stdin error:");
        return 0;
    };
    if(close(1) == -1){
        perror("close stdout error:");
        return 0;
    };
    if(close(2) == -1){
        perror("close stderr error:");
        return 0;
    };

    //reopen and redirect standrad io streams to null directory
    freopen( "/dev/null", "w+", stdin );
    freopen( "/dev/null", "w+", stdout );
    freopen( "/dev/null", "w+", stderr );

    //open log file
    file_fd = open( "/home/mykhailo/github/c-examples/process-daemonization/log.txt", O_WRONLY|O_APPEND);
    if( file_fd == -1 ){
        perror("open log as daemon error:");
        return 0;
    }

    //get daemonized process parameters
    pid_daemon = getpid();
    pid_t ppid_daemon = getppid();
    sid_daemon = getsid(pid_daemon);
    pid_t tid_daemon = syscall(SYS_gettid);
    uid_t uid_daemon = getuid();
    gid_t gid_daemon = getgid();

    printf("\nDaemon process IDs:\n"); //goes to /dev/null
    printf("PID: %d\n", pid_daemon);
    printf("PPID: %d\n", ppid_daemon);
    printf("SID: %d\n", sid_daemon);
    printf("TID: %d\n", tid_daemon);
    printf("UID: %d\n", uid_daemon);
    printf("GID: %d\n\n", gid_daemon);

    //make a log entry
    char buffer[512];
    sprintf(buffer, "daemon PID is #%d\ndaemon PPID is #%d\ndaemon SID is #%d\ndaemon TID is #%d\ndaemon UID is #%d\ndaemon GID is #%d", pid_daemon, ppid_daemon, sid_daemon, tid_daemon, uid_daemon, gid_daemon);
    if( log_entry(file_fd, buffer) == -1 ){
        if( close(file_fd) == -1 ){
            perror("close error:");
        };
        return 0;
    }

    //enter infinite loop
    while(1){};

    return 0;
}
