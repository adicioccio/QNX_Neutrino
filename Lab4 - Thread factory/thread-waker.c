/* Thread waker that takes number input from user and wakes that any threads
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

sem_t *semaphore;

int main(void) {

	semaphore = sem_open("semProcess", 0);

	printf("PID = %d : Thread-waker process.\n", getpid());

	int wakeup = -1;
	int i = 0;

	do {
		printf("Enter how many threads you want to wake up or 0 to exit.\n");
		scanf("%d", &wakeup);
			for (i = 0; i < wakeup; i++) {
				sem_post(semaphore);
			}
	} while (wakeup != 0);

	sem_close(semaphore);

	return EXIT_SUCCESS;
}


