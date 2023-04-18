/* SIG Handler w Multiple Processes Part B
## Project is currently under development
## No issues as of now
## A+
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

volatile sig_atomic_t usr1Happened = 0;
void sigint_handler(int sig);
sigset_t set;
volatile pid_t parentPID;

int main(void) {

	pid_t pid;
	//int rv;
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


	printf("Parent PID = %d : Parent running...\n", getpid());
	//parentPID = getpid();
	for (int i = 0; i<num; i++) {
		switch (pid = fork()) {
			case -1:
				perror("fork");
				exit(1);
				break;
			case 0:
				while (1) {
					if (usr1Happened == 1) {
						break;
					}
					printf("Child PID = %d : Child running...\n", getpid());
					sleep(5);
				}
				printf("Child PID = %d : Child received USR1.\n", getpid());
				printf("Child PID = %d : Child exited.\n", getpid());

				break;
			default:
				break;

		}
	}

	if(pid!=0) {
		while(num > 0) {
			pid_t pp = wait(NULL);
			while(pp > 0) num--;

		}
	}
	printf("PID = %d: Children finished, parent exiting.\n", getpid());
	return 0;
}


void sigint_handler(int sig) {
	printf("PID = %d : Received USR1. (%d)\n", getpid(), parentPID);
	//if (getpid() != parentPID) {
	usr1Happened = 1;
	//}

}
