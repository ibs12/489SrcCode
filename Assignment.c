#include <stdio.h>

#include <stdlib.h>

#include <sys/socket.h>

#include <strings.h>

#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>

#include <netdb.h>

#include <unistd.h>



#include <stdio.h>

#include <stdlib.h>

#include <sys/socket.h>

#include <strings.h>

#include <string.h>

#include <arpa/inet.h>

#include <sys/types.h>

#include <netdb.h>

#include "Commands.h"

#include "Server.h"

#include "Client.h"



#define TRUE 1

#define MSG_SIZE 256

#define BUFFER_SIZE 256









void *get_in_addr(struct sockaddr *sa) {



    if (sa->sa_family == AF_INET) {

        return &(((struct sockaddr_in*)sa)->sin_addr);

    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);

}



int main(int argc, char **argv) {



	int sock;

	

	int rv;

	

	struct sockaddr_in googleAddr;

	

	char Message[BUFFER_SIZE];



	struct sockaddr_in localAddr;

	

	socklen_t addrSize;



	struct addrinfo hints, *servinfo;

	if(argc<3){

		printf("Invalid Arguments Provided\n");

		return 1;

	}

	else if (strcmp(argv[1],"s")==0){

		

		printf("Trying to start Server\n");

		int Port= atoi(argv[2]);

		Create_Server(Port);

	}

	

	else if(strcmp(argv[1],"c")==0){

		printf("Trying to start Client\n");

		int Port= atoi(argv[2]);

		create_client_socket(Port);

	}

	else{

		printf("Invalid arguments provided");

	}

   	return 1;

    

}



