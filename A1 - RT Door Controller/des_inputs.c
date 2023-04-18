#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include "des-mva.h"

int main(int argc, char *argv[]) {

	// === PHASE I ===

	//Get controller's PID from command-line arguments.
	if (argc != 2) {
		//On Failure: print usage message and EXIT_FAILURE
		fprintf(stderr, "Usage: ./des_inputs [ControllerPID]\n");
		return EXIT_FAILURE;
	}
	pid_t controlPID = atoi(argv[1]);

	// Call ConnectAttach() to attach to controller's channel
	int coid = ConnectAttach(ND_LOCAL_NODE, controlPID, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		// On Failure: print error message and EXIT_FAILURE
		fprintf(stderr, "Connect Attach failed. Can't connect to controller PID.\n");
		return EXIT_FAILURE;
	}

	// === PHASE II ===
	char employeeID[128];
	char employeeWeight[128];
	char returnMsg[128];
	while (1) {
		// Prompt User for DES input-event (printf())
		puts("Enter the event type (ls= left scan, rs= right scan, ws= weight scale, lo =left open, ro=right open, lc = left closed, rc = right closed , gru = guard right unlock, grl = guard right lock, gll=guard left lock, glu = guard left unlock): ");
		char userInput[128];
		// Get input-event from User (scanf())
		scanf("%s", userInput);

		//IF input-event == "ls" THEN
		if (!strcmp(userInput, "ls")) {
			// Prompt User for person ID
			puts("(LS) Please enter employee ID: \n");
			// Get person ID from User
			scanf("%s", employeeID);
			printf("Person has been scanned with ID: %s\n", employeeID);
		}

		//TODO: apply what you know and repeat for "rs"
		if (!strcmp(userInput, "rs")) {
			// Prompt User for person ID
			puts("(RS) Please enter employee ID: \n");
			// Get person ID from User
			scanf("%s", employeeID);
			printf("Person has been scanned with ID: %s\n", employeeID);
		}
		//TODO: apply what you know and repeat for "ws", with the notable exception: prompt User for person's weight
		if (!strcmp(userInput, "ws")) {
			puts("(WS) Please enter the employee's weight: \n");
			scanf("%s", employeeWeight);
			printf("Person has been weighed at: %s\n", employeeWeight);
		}
		// Send Person object to controller (server); no message from controller is returned to client.
		MsgSend (coid, &userInput, sizeof (userInput), &returnMsg, sizeof (returnMsg));
		// IF input-event == "exit" THEN
		if (!strcmp(userInput, "exit")) {
			puts("Exiting Inputs...\n");
			//break out of while loop
			break;
		}
	}
	// === PHASE III ===
	ConnectDetach(coid);
	return EXIT_SUCCESS;
}
