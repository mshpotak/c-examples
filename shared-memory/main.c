// by Mykhailo SHPOTAK, Ivan PANCHENKO
// https://github.com/mshpotak/c-examples/shared-memory

//signal management
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
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

#ifdef _POSIX_MAPPED_FILES
    void * mmap(void *start, size_t length, int prot , int flags, int fd, off_t offset);
    int munmap(void *start, size_t length);
#endif

#define STR_SIZE 256
#define FILE2MODE 0644

//description of the marker datum
struct datum {
    int pid;
    long int timestamp;
    char str[STR_SIZE];
};

//function to write into the shared memory
int write_shm(int fd_shm, struct datum *shmem_addr, int shm_size){
    int result;
    time_t curtime;

    //synchronize a file with a memory map
    result = msync(shmem_addr, shm_size, MS_SYNC);
    if(result == -1){
        perror("write_shm() msync() error:");
    }

    //write a string into the shared memory
    printf("Input a string:\t");
    while(1){
        result = scanf("%[^\n]s", shmem_addr->str);
        if(result == -1){
            perror("write_shm() scanf() error:");
            return -1;
        }
        if(result > STR_SIZE){
            printf("The string is too big. The number of symbols must be less then %d.\n", STR_SIZE);
            printf("Input another string:\t");
            continue;
        } else break;
    }
    shmem_addr->pid = getpid();
    time(&curtime);
    shmem_addr->timestamp = curtime;

    result = write(fd_shm, shmem_addr,shm_size);
    if(result == -1){
        perror("write_shm() write() error:");
        return -1;
    }
    //print a datum about the written string
    printf("\nNew datum:\n\tPID:\t\t%d\n\tTimestamp:\t%.24s\n\tUser input:\t%s\n\n", shmem_addr->pid, ctime(&shmem_addr->timestamp), shmem_addr->str);
    while((getchar()) != '\n');
    getchar();

    return 0;
}

//function to read from the shared memory
int read_shm(int fd_shm, struct datum *shmem_addr, int shm_size){
    int result;

    //synchronize a file with a memory map
    result = msync(shmem_addr, shm_size, MS_SYNC);
    if(result == -1){
        perror("read_shm() msync() error:");
        return -1;
    }

    //read a string from the shared memory
    result = read(fd_shm, shmem_addr, shm_size);
    if(result == -1){
        perror("read_shm() read() error:");
        return -1;
    }
    //print a datum about the read string
    printf("Datum:\n\tPID:\t\t%d\n\tTimestamp:\t%.24s\n\tUser input:\t%s\n\n", shmem_addr->pid, ctime(&shmem_addr->timestamp), shmem_addr->str);
    getchar();

    return 0;
}

//function that reads from and writes to shared memory
int rdwr_shm(int fd_shm, struct datum *shmem_addr, int shm_size){
    int result;
    time_t curtime;

    printf("\033c");

    //read a string from the shared memory
    result = read_shm(fd_shm, shmem_addr, shm_size);
    if(result == -1){
        perror("read_shm() error:");
        return -1;
    }
    //write a new string into the shared memory
    result = write_shm(fd_shm, shmem_addr, shm_size);
    if(result == -1){
        perror("write_shm() error:");
        return -1;
    }
    return 0;
}

int main(int argc, const char *argv[]){

    int result;
    struct datum *shmem_addr;
    int fd_shm;
    const int datum_size = sizeof(struct datum);
    int (*argfunc)(int, struct datum*, int) = NULL;

    //register a shared memory file
    //int shm_open(const char *name, int oflag, mode_t mode);
    fd_shm = shm_open("/shmlog.txt", O_CREAT|O_RDWR, FILE2MODE);
    if(fd_shm == -1){
        perror("shm_open() error:");
    }
    //truncate the size of the shared memory file to the size of the datum
    result = ftruncate(fd_shm, datum_size);
    if(result == -1){
        perror("ftruncate() error:");
    }
    //initialize a pointer to the shared memory datum structure
    shmem_addr = (struct datum* ) mmap(NULL, datum_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if( shmem_addr == (void *) -1){
        perror("ftruncate() error:");
    }

    //check the number of arguments
    if(argc == 2){
        if(!strcmp(argv[1],"-r")) argfunc = &read_shm;
        else if(!strcmp(argv[1],"-w")) argfunc = &write_shm;
        else if(!strcmp(argv[1],"-rw")) argfunc = &rdwr_shm;
        else printf("error: nonexistent argument");
    }else if(argc == 1){
        argfunc = &rdwr_shm;
    }else{
        printf("error: too many arguments.");
        return -1;
    }

    //an infinite loop for editing the datum in the shared memory
    while(1){
        result = (*argfunc)(fd_shm, shmem_addr, datum_size);
        if(result == -1){
            continue;
        }
    }

    return 0;
}
