/* Calc Client that connects to the server and sends a payload including variables to computate and the receives the answer
## Project finished
## No issues
## A+
*/

#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include "calc_message.h"

int main(int argc, char* argv[]) {
	//variables (require other vars. too)
	client_send_t msg_send;
	server_response_t msg_receive;
	int coid;
	pid_t server_pid;

	//Validate number of command-line arguments
	if ( argc != 5 ) {
		//	Print usage message
		printf("[Usage: ./calc_client (SERVER PID) NUM1 OPERATOR NUM2]\n");
		return EXIT_FAILURE;
	}

	//Use ‘C’s atoi() function to convert command-line args from string (char *) to int
	msg_send.left_hand = atoi( argv[2] );
	//repeat for right_hand operand
	msg_send.right_hand = atoi( argv[4] );
	//repeat for server’s PID
	server_pid = atoi( argv[1] );
	//get operator (remember it’s a char) from command-line

	// Process msg_receive from calc_server
	if (atoi( argv[4] ) == 0) {
		printf("SRVR_UNDEFINED (cannot divide by 0)\n");
		return EXIT_FAILURE;
	}

	if ((atoi( argv[2] ) >= 10000) || (atoi( argv[4] ) >= 10000)) {
		printf("SRVR_OVERFLOW (handle overflow) on Client-side\n");
		return EXIT_FAILURE;
	}

	char *op;
	op = argv[3];
	switch(*op) {
		case '+' :
			msg_send.operator = '+';
			break;
		case '-':
			msg_send.operator = '-';
			break;
		case 'x':
			msg_send.operator = 'x';
			break;
		case '/':
			msg_send.operator = '/';
			break;
		default:
			printf("SRVR_INVALID_OPERATOR (handle unsupported operator)\n");
			return EXIT_FAILURE;
	}

	coid = ConnectAttach(ND_LOCAL_NODE, server_pid, 1, _NTO_SIDE_CHANNEL, 0);
	if (coid == -1) {
			fprintf(stderr, "Connect Attach failed. Can't connect to entered PID.\n");
			return EXIT_FAILURE;
		}
	// PHASE II: calc_client sends message to calc_server
	MsgSend( coid, &msg_send, sizeof(msg_send), &msg_receive, sizeof(msg_receive) );

	//IF SRV_OK: print expression = answer. Eg. The server has calculated the result of 2 + 3 = 5
	if (msg_receive.statusCode == SRVR_OK) {
		printf("Server has calculated the result of %d %c %d ", msg_send.left_hand, msg_send.operator, msg_send.right_hand);
		printf("as %.2f\n", msg_receive.answer);
		return EXIT_SUCCESS;
	}


	//IF ! SRV_OK: print error code and human-friend error message.
	if (msg_receive.statusCode != SRVR_OK) {
		printf("Server did not return SRVR_OK. Server side issue.");
		return EXIT_FAILURE;
	}

	//Phase III
	//Call ConnectDetach() to detach from server’s channel.
	ConnectDetach(coid);
	return EXIT_SUCCESS;
}
