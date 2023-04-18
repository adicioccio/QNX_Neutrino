/* Calc Server that creates a channel and communicates with the client in order to computate arguments
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
#include "calc_message.h"

int main(int argc, char* argv[]) {
	client_send_t client_message;
	server_response_t response;
	int rcvid;
	int chid;

	//TODO: implement Phase I (server)
	// create a channel --- Phase I
	chid = ChannelCreate(0);
	if (chid == -1) {
		perror("failed to create the channel.");
		exit(EXIT_FAILURE);
	}
	printf("PID = %d : Server started. \n", getpid());
	printf("CHID = %d : Channel created. \n", chid);


	// PHASE II: receive message from calc_client
	while (1) {
	    rcvid = MsgReceive( chid, &client_message, sizeof(client_message), NULL );
	    // Process message from calc_client:

	    //Assume calculation is OK response.statusCode = SRVR_OK;
	    switch( client_message.operator ) {
	    	case '+':
				response.answer = client_message.left_hand + client_message.right_hand;
				response.statusCode = SRVR_OK;
				break;
	    	case '-':
	    		response.answer = client_message.left_hand - client_message.right_hand;
	    		response.statusCode = SRVR_OK;
	    		break;
	    	case 'x':
	    		response.answer = client_message.left_hand * client_message.right_hand;
	    		response.statusCode = SRVR_OK;
	    		break;
	    	case '/':
	    		response.answer = client_message.left_hand / client_message.right_hand;
	    		response.statusCode = SRVR_OK;
	    		break;
	    } //end of switch

	    //End PHASE II of server: reply to client, which is currently blocked.
	    //snprintf(message, 50, "%.2f", response.answer);

	    MsgReply( rcvid, EOK, &response, sizeof(response) );

	// end while()
	}
	//TODO: implement Phase III (server)
	ChannelDestroy(chid);
	return EXIT_SUCCESS;
}
