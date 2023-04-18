#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include "des-mva.h"

int main(void) {
	// === PHASE I ===
	int chid;
	int rcvid;
	char receiveMsg[128];
	char sendMsg[128];
	// Call ChannelCreate() to create a channel for the controller process to attach
	chid = ChannelCreate(0);
	// Print display's PID; controller needs to know this PID
	printf("PID = %d : Display started. \n", getpid());

	// === PHASE II ===
	while(1) {
		// Call MsgReceive() to receive Display object from controller
		rcvid = MsgReceive(chid, &receiveMsg, sizeof(receiveMsg), NULL );
		MsgReply( rcvid, EOK, &sendMsg, sizeof(sendMsg) );

		// IF message == EXIT THEN
		if(strcmp(receiveMsg, "exit") == 0) {
			puts("Exiting Display...");
			// break out of while loop
			break;
		}
		// IF message == ID_SCAN THEN
		if(strcmp(receiveMsg, "ls") == 0) {
			// Print person has been scanned entering (or leaving) the building and display the person's ID
			printf("%s\n", outMessage[5]);
		}
		// IF message == ID_SCAN THEN
		if(strcmp(receiveMsg, "rs") == 0) {
			// Print person has been scanned entering (or leaving) the building and display the person's ID
			printf("%s\n", outMessage[5]);
		}
		// ELSE IF message = WEIGHED THEN
		if(strcmp(receiveMsg, "ws") == 0) {
			// Print person has been weighed and their weight
			printf("%s\n", outMessage[6]);
		// ELSE - regular message
		} else {
			// Print message to Neutrino console (i.e. stdout)
			printf("%s\n", receiveMsg);
		}

	}
	// === PHASE III ===
	ChannelDestroy(chid);
	return EXIT_SUCCESS;
}
