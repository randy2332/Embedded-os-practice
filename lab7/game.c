#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

typedef struct {
    int guess;
    char result[8];
}data;
data *shm;
# define SHMSZ sizeof(data)

// global variable
int key_int ;
int guess_truth;
int shmid;
int flag=1;



void handler(int signo,siginfo_t *info,void *context){
    // do some work
    if (shm->guess < guess_truth){
        strcpy(shm->result,"larger");
        printf("[game] Guess %d, %s\n",shm->guess,shm->result);
    }
    else if (shm->guess > guess_truth){
    	strcpy(shm->result,"smaller");
    	printf("[game] Guess %d, %s\n",shm->guess,shm->result);
    }
    else{
        strcpy(shm->result,"bingo");
        printf("[game] Guess %d, bingo\n",shm->guess);
    }
    printf("pid %d",info->si_pid);
    kill(info->si_pid,signo);
   
}

// 處理 Ctrl+C 信號，釋放共享記憶體
void cleanup() {
    //detach share memory segment
    shmdt(shm);
    /* Destroy the share memory segment */
    printf("Server destroy the share memory.\n");
    int retval = shmctl(shmid, IPC_RMID, NULL);
    if (retval < 0){
        fprintf(stderr, "Server remove share memory failed\n");
	exit(1);
    }
    flag = 0;
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "wrong type ,please use: %s <key> <guess>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
     // show pid so that we can manul enter to guess terminal
    pid_t pid =getpid();
    printf("[game] Game PID: %d\n",pid);
    
     // stored key and  guess number
    sscanf(argv[1],"%d",&key_int);
    sscanf(argv[2],"%d",&guess_truth);
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
    
    /* register handler to SIGUSR1 ,receive signal from guess*/ 
    struct sigaction my_action;
    memset(&my_action, 0, sizeof (struct sigaction));
    my_action.sa_flags = SA_SIGINFO;
    my_action.sa_sigaction = handler;
    sigaction(SIGUSR1, &my_action, NULL);
    
    //ctrl +c 
    signal(SIGINT,cleanup);
    
    while (flag){
        sleep(1);
    }
    

    return 0;
}


