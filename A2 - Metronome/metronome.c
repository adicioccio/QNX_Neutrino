#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

#define PAUSE_NUM 1
#define QUIT_NUM 2
#define PULSE_NUM 3
#define START_NUM 4
#define STOP_NUM 5
#define SET_NUM 6

char data[255];
int metronome_coid;
name_attach_t * attach;
int running = 1;

int timeSignatureTop;
int timeSignatureBottom;
int numOfIntervals;

typedef struct inputmessage {
	int timeSignatureTop;
	int timeSignatureBottom;
	int numOfIntervals;
}inputmessage ;

typedef union {
	struct _pulse pulse;
	char msg[255];
} my_message_t;

typedef struct DataTableRow {
	int timeSignatureTop;
	int timeSignatureBottom;
	int numOfIntervals;
	char patternEachBeat[32];
};
struct DataTableRow t[] = {
		{2, 4, 4, "|1&2&"},
		{3, 4, 6, "|1&2&3&"},
		{4, 4, 8, "|1&2&3&4&"},
		{5, 4, 10, "|1&2&3&4-5-"},
		{3, 8, 6, "|1-2-3-"},
		{6, 8, 6, "|1&a2&a"},
		{9, 8, 9, "|1&a2&a3&a"},
		{12, 8, 12, "|1&a2&a3&a4&a"}
};

// io_read() copy and paste from Lab7
int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
	//static char data[] = "hello";
		int nb;
		if (data == NULL) return 0;
		nb = strlen(data);

		//test to see if we have already sent the whole message.
		if (ocb->offset == nb)
			return 0;

		//We will return which ever is smaller the size of our data or the size of the buffer
		nb = min(nb, msg->i.nbytes);

		//Set the number of bytes we will return
		_IO_SET_READ_NBYTES(ctp, nb);

		//Copy data into reply buffer.
		SETIOV(ctp->iov, data, nb);

		//update offset into our data used to determine start position for next read.
		ocb->offset += nb;

		//If we are going to send any bytes update the access time for this resource.
		if (nb > 0)
			ocb->attr->flags |= IOFUNC_ATTR_ATIME;

		return(_RESMGR_NPARTS(1));
}

// * io_write()
int io_write(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
	 int nb = 0;

	if( msg->i.nbytes == ctp->info.msglen - (ctp->offset + sizeof(*msg) )) {
	/* have all the data */
	char *buf;
	char *alert_msg;
	int i, small_integer;
	buf = (char *)(msg+1);

	if(strstr(buf, "pause") != NULL) {
		for(i = 0; i < 2; i++) {
			alert_msg = strsep(&buf, " ");
		}
		small_integer = atoi(alert_msg);
		if(small_integer >= 1 && small_integer <= 9) {
			//FIXME :: replace getprio() with SchedGet()
			MsgSendPulse(metronome_coid, SchedGet(0,0,NULL), PAUSE_NUM, small_integer);
		} else {
			printf("\nInteger is not between 1 and 9.\n");
		}
	} else if(strstr(buf, "set") != NULL) {
		alert_msg = strsep(&buf, " ");
		char* numIntervals = strsep(&buf, " ");
		char* sigTop = strsep(&buf, " ");
		char* sigBottom = strsep(&buf, " ");

		numOfIntervals = atoi(numIntervals);
		timeSignatureTop = atoi(sigTop);
		timeSignatureBottom = atoi(sigBottom);

		MsgSendPulse(metronome_coid, SchedGet(0,0,NULL), SET_NUM, small_integer);
	} else if(strstr(buf, "quit") != NULL) {
		MsgSendPulse(metronome_coid, SchedGet(0,0,NULL), QUIT_NUM, small_integer);
	} else if(strstr(buf, "start") != NULL) {
		MsgSendPulse(metronome_coid, SchedGet(0,0,NULL), START_NUM, small_integer);
	} else if(strstr(buf, "stop") != NULL) {
		MsgSendPulse(metronome_coid, SchedGet(0,0,NULL), STOP_NUM, small_integer);
	} else {
		printf("\nPlease enter correct input!\n");
		strcpy(data, buf);
	}

	nb = msg->i.nbytes;
	}
	_IO_SET_WRITE_NBYTES (ctp, nb);

	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS (0));
}

// * io_open()
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
	if ((metronome_coid = name_open( "metronome", 0 )) == -1) {
	        perror("name_open failed.");
	        return EXIT_FAILURE;
	    }
	    return (iofunc_open_default (ctp, msg, handle, extra));
}

void metronome_thread(void *arg) {
	timer_t timer_id;
	struct itimerspec itimer;

	struct sigevent event;
	my_message_t msg;
	int rcvid;

	inputmessage* userInput = (inputmessage*) arg;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_priority = SchedGet(0,0,NULL);
	event.sigev_code = PULSE_NUM;
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, attach->chid, _NTO_SIDE_CHANNEL, 0);

	timer_create(CLOCK_REALTIME, &event, &timer_id);

	// ex. 60 sec / 120 beats = 0.5 sec / beat
	double timePerBeat = (double)60/numOfIntervals;
	// ex. (0.5 sec * 2) / 4 = 0.25 sec interval in between each |, 1, &, 2
	double interval =  (double)(timePerBeat*2)/timeSignatureBottom;

	itimer.it_interval.tv_sec = interval;
	itimer.it_interval.tv_nsec = (interval * 1e+9);
	itimer.it_value.tv_sec = 0;
	itimer.it_value.tv_nsec = 500000000;

	timer_settime(timer_id, 0, &itimer, NULL);

	int index = 0;
	for(int i = 0; i < 8; ++i) {
		if (t[i].timeSignatureTop == userInput->timeSignatureTop && t[i].timeSignatureBottom == userInput->timeSignatureBottom) {
			index = i;
			break;
		}
	}

	// Phase II - receive pulses from interval timer OR io_write(pause, quit)
	for (;;) {
		char *ptr;
		ptr=t[index].patternEachBeat;
		while(1) {
			rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
			if ( rcvid == 0 ){
				switch(msg.pulse.code) {
					case PULSE_NUM:
						printf("%c", *ptr++);
						break;
					case PAUSE_NUM:
						itimer.it_value.tv_sec = msg.pulse.value.sival_int;
						timer_settime(timer_id, 0, &itimer, NULL);
						break;
					case START_NUM:
						if (!running) {
							printf("\nStarting...\n");
							itimer.it_value.tv_nsec = 500000000;
							timer_settime(timer_id, 0, &itimer, NULL);
							running = 1;
						} else {
							printf("Already running!\n");
						}
						break;
					case STOP_NUM:
						if (running) {
							printf("\nStopping...\n");
							itimer.it_value.tv_nsec = 0;
							timer_settime(timer_id, 0, &itimer, NULL);
							running = 0;
						} else {
							printf("Already stopped!\n");
						}
						break;
					case SET_NUM:
						userInput->numOfIntervals = numOfIntervals;
						userInput->timeSignatureTop = timeSignatureTop;
						userInput->timeSignatureBottom = timeSignatureBottom;
						double timePerBeat1 = (double)60/numOfIntervals;
						double interval1 =  (double)(timePerBeat1*2)/timeSignatureBottom;
						itimer.it_interval.tv_sec = interval1;
						itimer.it_interval.tv_nsec = (interval1 * 1e+9);
						int index1 = 0;
						for(int i = 0; i < 8; ++i) {
							if (t[i].timeSignatureTop == userInput->timeSignatureTop && t[i].timeSignatureBottom == userInput->timeSignatureBottom) {
								index1 = i;
								break;
							}
						}
						printf("%d", index1);
						timer_settime(timer_id, 0, &itimer, NULL);
						index = index1;
						break;
					case QUIT_NUM: // implement Phase III:
						//  delete interval timer
						TimerDestroy(timer_id);
						//  call name_detach()
						name_detach(attach, 0);
						//  call name_close()
						name_close(attach);
						//  exit with SUCCESS
						printf("\nExiting...\n");
						exit(EXIT_SUCCESS);
						break;
				}
				if(*ptr == '\0'){
					printf("\n");
					break;
				}
			}
			fflush(stdout);
		}
	}
}

int main(int argc, char *argv[]) {
	// verify number of command-line arguments == 4
	if (argc != 4) {
		// then usage message and exit with FAILURE
		fprintf(stderr, "Usage: ./metronome [timeSignatureTop] [timeSignatureBottom] [numOfIntervals]\n");
		return EXIT_FAILURE;
	}
	// process the command-line arguments:
	timeSignatureTop = atoi(argv[2]);
	timeSignatureBottom = atoi(argv[3]);
	numOfIntervals = atoi(argv[1]);
	struct inputmessage userInput = {
		timeSignatureTop,
		timeSignatureBottom,
		numOfIntervals,
	};

	// implement main(), following simple_resmgr2.c and Lab7 as a guide
	dispatch_t* dpp;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	iofunc_attr_t ioattr;
	dispatch_context_t *ctp;
	int id;

	// Phase I - create a named channel to receive pulses
	attach = name_attach(NULL, "metronome", 0);

	if ((dpp = dispatch_create ()) == NULL) {
		fprintf (stderr, "%s:  Unable to allocate dispatch context.\n", argv [0]);
		return (EXIT_FAILURE);
	}
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
	connect_funcs.open = io_open;
	io_funcs.read = io_read;
	io_funcs.write = io_write;

	iofunc_attr_init(&ioattr, S_IFCHR | 0666, NULL, NULL);
	// device path (FQN): /dev/local/metronome
	if ((id = resmgr_attach (dpp, NULL, "/dev/local/metronome",
					_FTYPE_ANY, 0, &connect_funcs, &io_funcs,
					&ioattr)) == -1) {
		fprintf (stderr, "%s:  Unable to attach name.\n", argv [0]);
		return (EXIT_FAILURE);
	}
	// create the metronome thread in-between calling resmgr_attach() and while(1) { ctp = dispatch_block(... }
	pthread_attr_t a;
	pthread_attr_init(&a);
	pthread_create(NULL, &a, &metronome_thread, &userInput);

	ctp = dispatch_context_alloc(dpp);
	while(1) {
		ctp = dispatch_block(ctp);
		dispatch_handler(ctp);
	}
	return EXIT_SUCCESS;

}
