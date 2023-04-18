/* SIG Handler Part A
## Project is currently under development
## No issues as of now
## A+
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

volatile sig_atomic_t usr1Happened = 0;
void sigint_handler(int sig);
sigset_t set;

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
	printf("PID = %d : Running.\n", getpid());
	while (1) {
		if (usr1Happened == 1) {
			break;
		}
		sleep(5);
	}

	printf("PID = %d : Exiting.\n", getpid());
	return EXIT_SUCCESS;
}

void sigint_handler(int sig) {
	printf("PID = %d : Received USR1.\n", getpid());
	usr1Happened = 1;
}
