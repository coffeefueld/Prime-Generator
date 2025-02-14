#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/wait.h>

int primeCheck(unsigned long long number) { //Function that takes a number as input and checks if the number is a prime
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
    ncpu = sysconf(_SC_NPROCESSORS_ONLN); // Find out number of total virtual processing cores (cpu threads) in the running system.
    // Variables used to store big numbers
    long long unsigned int numbers = 0; 
    long long unsigned int start;
    long long unsigned int end;
    long long unsigned int lim;
    
    int fdin[2];
    if(pipe(fdin) < 0){ // Creation of a pipe
        perror("pipe");
        exit(2);
    }
    
    /* MENU */

    printf("How many threads would you like the program to use (recommended 4, you have %ld threads available): ", ncpu);
    scanf("%d", &threads);
    if(threads > ncpu || threads < 0) { // Checks if the user options are compatible with the system
        printf("Error: incorrect amount of threads\nYour system supports up to %ld threads, try something less than that.\n", ncpu);
        exit(-1);
    }
    printf("Which number should the program start from: ");
    scanf("%llu", &start);
    printf("How many numbers after %llu should the program check: ", start);
    scanf("%llu", &end);
    lim = start + end;

    /* Main Program */
    
    pid_t p[threads];
    for (i = 0; i< threads;i++) {
        p[i] = fork(); //Array of PIDs
        if (p[i] == 0)
        {
            /* child */

            close(fdin[1]); // Closes unised pipe
            
            while(1){
                usleep(100); // Sleeps to make sure the program doesng hang in the end. If your execution hangs in the end inclrease by +100 untill it doesn't
                if(poll(&(struct pollfd){ .fd = fdin[0], .events = POLLIN }, 1, 2000) == 0) { // Checks to see if the pipe is empty, if so the child terminates 
                    break;
                }
                read(fdin[0], &numbers, sizeof(numbers)); // Reads from pipe

                if(primeCheck(numbers)){ // Checks if the number read from the pipe is a prime and if so it prints it to STDOUT
                    printf("%llu\n", numbers);
                }
            }
            close(fdin[0]); // Closes pipe
            exit(0);
        }
    }
        
    /* parent*/

    close(fdin[0]); // Closes unused pipe
    
    for(numbers = start; numbers < lim; numbers++){ // Writes the numbers in the pipe
        write(fdin[1], &numbers, sizeof(numbers));
    }
    for(i = 0;i<threads;i++) { // Waits for all the children to finish
        wait(NULL);
    } 
    printf("Finishing Up...\n");
    close(fdin[1]); // Closes pipe
}
