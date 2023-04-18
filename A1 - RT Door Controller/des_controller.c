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
	int rcvid;
	int chid;
	int msgRply = 0;
	int state = START_STATE;
	int rightdoor = LOCKED;
	int leftdoor = LOCKED;
	char receiveMsg[128];
	char returnMsg[128];
	// Call ChannelCreate() to create a channel for the inputs process to attach
	chid = ChannelCreate(0);
	// Print controller's PID; inputs needs to know this PID
	printf("PID = %d : Controller started. \n", getpid());

	//Get display's PID from command-line arguments.
	if (argc != 2) {
		//On Failure: print usage message and EXIT_FAILURE
		fprintf(stderr, "Usage: ./des_controller [DisplayPID]\n");
		return EXIT_FAILURE;
	}
	pid_t displayPID = atoi(argv[1]);
	// Call ConnectAttach() to attach to display's channel
	int coid = ConnectAttach(ND_LOCAL_NODE, displayPID, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
		// On Failure: print error message and EXIT_FAILURE
		fprintf(stderr, "Connect Attach failed. Can't connect to display PID.\n");
		return EXIT_FAILURE;
	}

	state = READY_STATE;

	// === PHASE II ===
	while (1) {
		// receive the command from inputs
		rcvid = MsgReceive( chid, &receiveMsg, sizeof(receiveMsg), NULL );

		// IF message == EXIT THEN
		if(strcmp(receiveMsg, "exit") == 0) {
			puts("Exiting Controller...");
			state = EXIT_STATE;
			MsgSend (coid, &receiveMsg, sizeof (receiveMsg), &returnMsg, sizeof (returnMsg));
			// break out of while loop
			break;
		}
		// IF message == LS set to punch state
		if(strcmp(receiveMsg, "ls") == 0) {
			state = PUNCH_STATE;
		}
		// IF message == RS set to punch state
		if(strcmp(receiveMsg, "rs") == 0) {
			state = PUNCH_STATE;
		}
		// IF message == WS set to punch state
		if(strcmp(receiveMsg, "ws") == 0) {
			state = WEIGH_STATE;
		}
		if(strcmp(receiveMsg, "gll") == 0) {
			state = ARMED_STATE;
			if (leftdoor == LOCKED) {
				strcpy(receiveMsg, "Left door is already locked!");
			} else if (leftdoor == UNLOCKED_OPEN) {
				strcpy(receiveMsg, "Please close left door before locking!");
				state = LEFT_DOWN_STATE;
			} else {
				strcpy(receiveMsg, outMessage[4]);
				leftdoor = LOCKED;
			}
		}
		if(strcmp(receiveMsg, "glu") == 0) {
			state = LEFT_DOWN_STATE;
			if (leftdoor == LOCKED) {
				strcpy(receiveMsg, outMessage[2]);
				leftdoor = UNLOCKED_CLOSED;
				state = ARMED_STATE;
			} else {
				strcpy(receiveMsg, "Left door is already unlocked!");
			}
		}
		if(strcmp(receiveMsg, "lc") == 0) {
			if (leftdoor == UNLOCKED_CLOSED) {
				strcpy(receiveMsg, "Left door is already closed!");
			} else if (leftdoor == UNLOCKED_OPEN) {
				strcpy(receiveMsg, "Closing left door!");
				leftdoor = UNLOCKED_CLOSED;
			} else {
				strcpy(receiveMsg, "Can not open/close door while it is locked!");
			}
		}
		if(strcmp(receiveMsg, "lo") == 0) {
			if (leftdoor == UNLOCKED_OPEN) {
				strcpy(receiveMsg, "Left door is already open!");
			} else if (leftdoor == UNLOCKED_CLOSED) {
				strcpy(receiveMsg, "Opening left door!");
				leftdoor = UNLOCKED_OPEN;
			} else {
				strcpy(receiveMsg, "Can not open/close door while it is locked!");
			}
		}
		if(strcmp(receiveMsg, "grl") == 0) {
			state = ARMED_STATE;
			if (rightdoor == LOCKED) {
				strcpy(receiveMsg, "Right door is already locked!");
			} else if (rightdoor == UNLOCKED_OPEN) {
				strcpy(receiveMsg, "Please close right door before locking!");
				state = RIGHT_DOWN_STATE;
			} else {
				strcpy(receiveMsg, outMessage[4]);
				rightdoor = LOCKED;
			}
		}
		if(strcmp(receiveMsg, "gru") == 0) {
			state = RIGHT_DOWN_STATE;
			if (rightdoor == LOCKED) {
				strcpy(receiveMsg, outMessage[3]);
				rightdoor = UNLOCKED_CLOSED;
				state = ARMED_STATE;
			} else {
				strcpy(receiveMsg, "Right door is already unlocked!");
			}
		}
		if(strcmp(receiveMsg, "rc") == 0) {
			if (rightdoor == UNLOCKED_CLOSED) {
				strcpy(receiveMsg, "Right door is already closed!");
			} else if (rightdoor == UNLOCKED_OPEN) {
				strcpy(receiveMsg, "Closing right door!");
				rightdoor = UNLOCKED_CLOSED;
			} else {
				strcpy(receiveMsg, "Can not open/close door while it is locked!");
			}
		}
		if(strcmp(receiveMsg, "ro") == 0) {
			if (rightdoor == UNLOCKED_OPEN) {
				strcpy(receiveMsg, "Right door is already open!");
			} else if (rightdoor == UNLOCKED_CLOSED) {
				strcpy(receiveMsg, "Opening right door!");
				rightdoor = UNLOCKED_OPEN;
			} else {
				strcpy(receiveMsg, "Can not open/close door while it is locked!");
			}
		}
		// relay the command to the controller
		MsgSend (coid, &receiveMsg, sizeof (receiveMsg), &returnMsg, sizeof (returnMsg));

		// reply to inputs
		MsgReply (rcvid, EOK, &msgRply, sizeof(msgRply));

	}
	// === PHASE III ===
	ChannelDestroy(chid);
	ConnectDetach(coid);
	return EXIT_SUCCESS;
}
