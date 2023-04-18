#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

    typedef union {
        struct _pulse   pulse;
	char msg[255];
    } my_message_t;

int main(void) {
	char status[255];
	char value[255];
	my_message_t msg;
	//Configure myController as a server; register the device within the namespace

	//call name_attach() and register the device name: “mydevice”
	name_attach_t *nameAttach;
	//exit FAILURE if name_attach() failed
	if ((nameAttach = name_attach(NULL, "mydevice", 0)) == NULL) {
		return EXIT_FAILURE;
	}
	//Upon startup, myController is to read the status of the device:

	//call fopen() to open the device: /dev/local/mydevice
	FILE *fd = fopen("/dev/local/mydevice", "r");
	if (fd == NULL) {
		puts("ERROR in FOPEN.");
	}
	//scan the status and value from the file
	fscanf(fd, "%s %s", status, value);
	//if the status is “status” then printf %s, value
	if (strcmp(status, "status")) {
		printf("%s", value);
	}
//	//then if the value is “closed”
//	if (strcmp(value, "closed")) {
//		//then name_detach and exit with SUCCESS
//		name_detach(nameAttach, 0);
//		return EXIT_SUCCESS;
//	}
	//close the device by its fd
	fclose(fd);
	int rcvid;
	//while True:
	while(1) {
		//call MsgReceivePulse(attach->chid, &msg, sizeof(msg), NULL)
		rcvid = MsgReceivePulse(nameAttach->chid, &msg, sizeof(msg), NULL);
		//if received a pulse message (i.e. rcvid == 0)
		if (rcvid == 0) {
			//then if received MY_PULSE_CODE
			if (msg.pulse.code == MY_PULSE_CODE) {
				//then print the small integer from the pulse: printf("Small Integer: %d\n", msg.pulse.value.sival_int)
				printf("Small Integer: %d\n", msg.pulse.value.sival_int);
				//call fopen() to open the device: /dev/local/mydevice
				fd = fopen("/dev/local/mydevice", "r");
				//scan the status and value from the file
				fscanf(fd, "%s %s", status, value);
				printf("Status: %s\n", value);
				//then if the value is “closed”
				if (!strcmp(value, "closed")) {
					//then name_detach and exit with SUCCESS
					name_detach(nameAttach, 0);
					return EXIT_SUCCESS;
				}
				//close the fd
				fclose(fd);
			}
		//else //rcvid != 0
		} else {
			// display appropriate error message and exit with FAILURE
			puts("MSG RECEIVE PULSE FAILED");
			return EXIT_FAILURE;

		}
		MsgReply(rcvid, EOK, 0, 0);
	}
	//call name_detach()
	name_detach(nameAttach, 0);
	//exit with SUCCESS
	return EXIT_SUCCESS;
}
