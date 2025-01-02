#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/wait.h>

int primeCheck(unsigned long long number) {
    if (number < 2) return 0;
    for (unsigned long long i = 2; i * i <= number; i++) {
        if (number % i == 0) return 0;
    }
    return 1;
}


int main() {
    int i ;
    int threads;
    
    long ncpu;
    ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    
    long long unsigned int numbers = 0;
    long long unsigned int start;
    long long unsigned int end;
    long long unsigned int lim;
    
    int fdin[2];
    if(pipe(fdin) < 0){
        perror("pipe");
        exit(2);
    }
    
    /* MENU */

    printf("How many threads would you like the program to use (recommended 4): ");
    scanf("%d", &threads);
    if(threads > ncpu || threads < 0) {
        printf("Error: incorrect amount of threads\nYour system supports up to %ld threads, try something less than that.\n", ncpu);
        exit(-1);
    }
    printf("Which number should the program start from: ");
    scanf("%llu", &start);
    printf("How many numbers after %llu should the program check: ", start);
    scanf("%llu", &end);
    lim = start + end;
    
    pid_t p[threads];
    for (i = 0; i< threads;i++) {
        p[i] = fork();
        if (p[i] == 0)
        {
            /* child */

            close(fdin[1]);
            
            while(1){
                if(poll(&(struct pollfd){ .fd = fdin[0], .events = POLLIN }, 1, 2000) == 0) {
                    break;
                }
                read(fdin[0], &numbers, sizeof(numbers));

                if(primeCheck(numbers)){
                    printf("%llu\n", numbers);
                }
            }
            exit(0);
        }
    }
        
    /* parent*/

    close(fdin[0]);
    for(numbers = start; numbers < lim; numbers++){
        write(fdin[1], &numbers, sizeof(numbers));
    }

    for(i = 0;i<threads;i++) {
        wait(NULL);
    } 
}