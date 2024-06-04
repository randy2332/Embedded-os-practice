#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>


typedef struct {
    int guess;
    char result[8];
}data;
data *shm;
# define SHMSZ sizeof(data)

//global variable
int shmid;
int key_int;
int upper_bound;
int lower_bound =1;
int pid ;


void handler(int signo,siginfo_t *info,void *context){
    
    // do some work
    if (strcmp(shm->result,"larger")==0){
        lower_bound = shm->guess;
        
    }
    else if (strcmp(shm->result,"smaller")==0){
    	upper_bound = shm->guess;
    	
    }
    else{
        exit(0);// guess right!
    }
    
}

void timer_handler (int signum){
    // do some work
    shm->guess  = (upper_bound+lower_bound)/2;
    printf("[game] Guess: %d\n",shm->guess);
    printf("pid:%d",pid);
    kill(pid,SIGUSR1);
}
int main(int argc, char *argv[]){
    if (argc != 4) {
        fprintf(stderr, "wrong type ,please use: %s <key> <upper_bound <pid>>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    // read from user input
    sscanf(argv[1],"%d",&key_int);
    sscanf(argv[2],"%d",&upper_bound);
    sscanf(argv[3],"%d",&pid);
    printf("pid:%d\n",pid);
    key_t key = key_int;
    //shared memory
     /* Create the segment */
    if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    /* Now we attach the segment to our data space */
    if ((shm = shmat(shmid, NULL, 0)) == (data *) -1) {
        perror("shmat");
        exit(1);
    }
    //printf("sucess creat shared memory\n");
   /* register handler to SIGUSR1 */
    struct sigaction my_action;
    memset(&my_action, 0, sizeof (struct sigaction));
    my_action.sa_flags = SA_SIGINFO;
    my_action.sa_sigaction = handler;
    sigaction(SIGUSR1, &my_action, NULL);
    
    //printf("sucess creat SIGUSR1\n");
    
    /*time handler*/
    struct sigaction sa;
    struct itimerval timer;
    /* Install timer_handler as the signal handler for SIGVTALRM */
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;   
    sigaction (SIGVTALRM, &sa, NULL);
    /* Configure the timer to expire after 1000 msec */
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    /* Reset the timer back to 1000 msec after expired */
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;
    /* Start a virtual timer */
    setitimer (ITIMER_VIRTUAL, &timer, NULL);
    /* Do busy work */
    while (1);
    return 0;
}
