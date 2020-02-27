// by Mykhailo SHPOTAK, Ivan PANCHENKO
// https://github.com/mshpotak/c-examples/multiplex-io

//for open(), creat() and file management tools
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//standard input/output functions
#include <stdio.h>
//for read() and write()
#include <unistd.h>
//for perror() and errno
#include <errno.h>
//for explicit exit()
#include <stdlib.h>
//for multiplexing select()
#include <sys/select.h>
//for time management
#include <sys/time.h>

void user_exit(){
    printf("Do you want to continue (y/n)? \n");
    char input;

    answer:
    if((input = getchar()) == 'y'){
        printf("\n\n");
        return;
    } else if(input == 'n'){
        exit(-1);
    } else {
        goto answer;
    }
    return;
}

void print_error(){
    perror("Error:\t");
    user_exit();
    return;
}

int main(int argc, const char *argv[]){

    //define an identifier
    int id;
    if(argc > 2){
        printf("Error:\t Too many arguments.");
        return -1;
    //use user used identifier
    } else if(argc == 2){
        id = 1;
    //use program name as identifier
    } else id = 0;

    //select() structures
    fd_set read_set;
    struct timeval timeout_s;

    int buffer_size = 1024;
    char buffer[buffer_size];
    int sel_res;
    int bytes_rd;

    //start input loop
    while(1){
        //set arguments for select()
        FD_ZERO(&read_set);
        FD_SET(STDIN_FILENO, &read_set);
        timeout_s.tv_sec = 5;
        timeout_s.tv_usec = 0;

        //set select() for *readfds with *timeout of 5 seconds
        if((sel_res = select(STDIN_FILENO+1, &read_set, NULL, NULL, &timeout_s)) == -1) {
            print_error();
            continue;
        }
        printf("Select results:\t%d\n", sel_res);

        //if data is availbale read data <1024 bytes long
        if(sel_res > 0){
            if((bytes_rd = read(STDIN_FILENO, buffer, buffer_size)) == -1) {
                print_error();
                continue;
            }
            //print read data with an identifier
            printf("\n%s:\t%s\n\n", argv[id], buffer);
            user_exit();
        //if time has run out perror() identifier
        }else if(sel_res == 0){
            fprintf(stderr, "\nYour time is out: %s\n", argv[id]);
        }
    }

    return 0;
}
