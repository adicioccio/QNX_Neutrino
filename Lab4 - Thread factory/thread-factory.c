/* Thread Factory that produces semaphores
## Project finished
## No issues
## A+
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdbool.h>

volatile sig_atomic_t usr1Happened = 0;
sigset_t set;
sem_t *semaphore;
void sigint_handler(int sig);
void* child_process(void* arg);

int main(void) {
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0; // or SA_RESTART
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);

	if (sigaction(SIGUSR1, &sa, NULL) == -1) {
			perror("sigaction");
			exit(1);
	}

	int num = 0;
	puts("Enter the number of children:\n");
	scanf("%d", &num);

	semaphore = sem_open("semProcess", O_CREAT, S_IWOTH, 0);

	pthread_attr_t a;
	int i;
	for (i = 0; i < num; i++) {
		pthread_attr_init(&a);
		pthread_create(NULL, &a, &child_process, NULL);
		pthread_attr_destroy(&a);
	}

	while (usr1Happened == 0) {
		// parent process infinite loop until killed
	}

	sem_close(semaphore);
	sem_unlink(&semaphore);

	return EXIT_SUCCESS;
}

void* child_process(void* arg) {
	printf("PID = %d : Child created.\n", pthread_self());
	while (true) {
		int status = sem_wait(semaphore);
		if(status != 0){
			// error handling
			perror("Sem-wait unsuccessful.\n");
		}
		printf("PID = %d : Child finished with status %d.\n", pthread_self(), status);
		sleep(5);
	}
	return NULL;
}

void sigint_handler(int sig) {
	printf("PID = %d : Received USR1.\n", getpid());
	//if (getpid() != parentPID) {
	usr1Happened = 1;
	//}

}

