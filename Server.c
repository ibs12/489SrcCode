	#include "Server.h"

	#include <arpa/inet.h>

	#include <stdio.h>

	#include <string.h>

	#include <sys/socket.h>

	#include <unistd.h>

	#include <netinet/in.h>

	#include <stdlib.h>

	#include "../include/global.h"

	#include "../include/logger.h"

	#include "Commands.h"

	#include <netdb.h>

	#define CMD_SIZE 100

	#define BUFFER_SIZE 256



	int server_socket;



	struct sockaddr_in client_addr,server_addr;



	fd_set master_list, watch_list;



	int head_socket, selret, sock_index, fdaccept=0, caddr_len;



	int PORT = 8080;



	char buffer[256];



	typedef struct Client {



		char IPaddress[30];



		char Name[30];



		int ListeningPort;



		int FD;

		

		int MessagesReceived;

		

		int MessagesSent;

		

		int LoggedIn;

		

		char* BlockList[5];

		

		int NumberOfBlocked;

	} Client;



	typedef struct Message{

			char SourceIP[30];

			char Message[256];

		}Message;

	typedef struct Backlog {

		char DestIP[30];

		

		Message MessageList[5];

		

		int NumOfMessages;

	}Backlog;

	char* EmptyString[30];

	Client List[5];

	Backlog ListOfBacklogs[5];



	Backlog* EmptyLog;



	Message* Empty;



	Client* Dummy;



	char* BlockList[5];

	void AddToBacklog(char* SourceIP,char* DestIP,char* Message){

		struct Message NewMessage;

		strcpy(NewMessage.SourceIP,SourceIP);

		strcpy(NewMessage.Message, Message);

		int exists=0;

		for(int i=0; i<5; i++){

			struct Backlog ClientInList=ListOfBacklogs[i];

			if (strcmp(ClientInList.DestIP,DestIP)==0){

				exists=1;

				ClientInList.MessageList[ClientInList.NumOfMessages]=NewMessage;

				ClientInList.NumOfMessages+=1;		

			}

		}

		if (exists==0){

			struct Backlog NewBacklog;

			strcpy(NewBacklog.DestIP,DestIP);

			NewBacklog.MessageList[0]=NewMessage;

			NewBacklog.NumOfMessages=1;

			for(int i=0; i<5; i++){

				if (strcmp(ListOfBacklogs[i].DestIP,"69")==0){

					ListOfBacklogs[i]=NewBacklog;

					}

			}	

		}

	}



	void ClearBacklog(char* DestIP,char* Message){

		for(int i=0; i<5; i++){

			struct Backlog CurrentClient=ListOfBacklogs[i];

			if (strcmp(CurrentClient.DestIP,DestIP)==0){

				CurrentClient.NumOfMessages=0;

			}

			

		}

	}



	void LogClientOut(int socket){

		close(socket);

		FD_CLR(socket, &master_list);

		for (int i = 0; i < 5; i++) {

			if (List[i].FD == socket) {

				List[i].LoggedIn=0;

				List[i].FD=-1;

		}

	}

	}





	char* GetIPAddress(int client_fd) {

		printf("GET IP ADDRESS CALLED");

		char* Ip=malloc(100*sizeof(char));

		  for (int i = 0; i < 5; i++) {

		      if (List[i].FD == client_fd) {

		      		strcpy(Ip,List[i].IPaddress);

		          return Ip;

		      }

		  }

		  return NULL; // Client with the given FD not found

	}

void remove_connection(int socket) {



		close(socket);

		char* Ip=GetIPAddress(socket);

		

		FD_CLR(socket, &master_list);



		for (int i = 0; i < 5; i++) {



			if (List[i].IPaddress == Ip) {

				List[i]=*Dummy;

			}	



		}

	}

	void Parse1(char** Command,char** FirstArgPointer, char** SecondArgPointer, char* Actualmsg){

		

		int count=0;

		int iterator1=0;

		int iterator2=0;

		int iterator3=0;

		

		int j=strlen(Actualmsg);

		for (int i=0; i<strlen(Actualmsg); i++){

			char Character[1];

			strncpy(Character,&Actualmsg[i],1);

			Character[1]='\0';

			if(count==1){

				(*FirstArgPointer)[iterator2]=*Character;

				iterator2 ++;

			}

			//POSSIBLE ERRORS DUE TO MESSAGE SIZE 

			if ((count>1)&&(iterator3<256)){

				(*SecondArgPointer)[iterator3]=*Character;

				iterator3++;

			}

			if (strcmp(Character," ")==0){

				count++;



			}

			if (count==0){

				(*Command)[iterator1]=*Character;

				iterator1++;		

			}

			}

	}

	char* MessageCreator(char* Message,char* Command,char* SourceIP,char* DestIP, int success){

		printf("MESSAGE CREATOR CALLED\n");

		printf("success=%d\n",success);

		char *ReturnM = malloc(300*sizeof(char));

		if (success==1){

			sprintf(ReturnM,"[%s:SUCCESS]\n",Command);

			if (((((strcmp(Command,"LOGIN")==0)||(strcmp(Command,"SEND")==0))||(strcmp(Command,"BLOCK")==0))||(strcmp(Command,"UNBLOCK")==0))||				(strcmp(Command,"BROADCAST")==0)){

				sprintf(ReturnM+strlen(ReturnM),"[%s:END]\n",Command);

			}

			else if (strcmp(Command,"RELAYED")==0){

				printf("RELAYED");

				sprintf(ReturnM+strlen(ReturnM),"msg from:%s\n[msg]:%s\n[RELAYED:END]\n", SourceIP,Message);

			}

				

			}

		else{

		printf("ELSE\n");

			sprintf(ReturnM,"[%s:ERROR]\n[%s:END]\n",Command,Command);

		}	

		return ReturnM;

	}

	char* ReturnMessage(const Client LIST[]){



		char *ReturnM = malloc(1024);



		int id=1;



		for(int i=0; i<5; i++){

			int LoggedIn=List[i].LoggedIn;



			if (LoggedIn==1){



				sprintf(ReturnM+strlen(ReturnM), "%-5d%-35s%-20s%-8d\n",id,LIST[i].Name,LIST[i].IPaddress, LIST[i].ListeningPort);



				id+=1;



		}



		}

		return	ReturnM;



	}



	char* MessageForRelayed(char* IP,char* message){

	 	int length = snprintf(NULL, 0, "RELAYED msg from: %s\n[msg]: %s\n", IP, message);

	 	char* formattedMessage = (char*)malloc(length + 1);

	 	snprintf(formattedMessage, length + 1, "RELAYED msg from: %s\n[msg]: %s\n", IP, message);

		  	return formattedMessage;



	}

	int compareClients(const void *a, const void *b) {



		  const Client *clientA = (const Client *)a;



		  const Client *clientB = (const Client *)b;



		  return clientA->ListeningPort - clientB->ListeningPort;



	}



	int AddClient(char ip[], char Name[], int LP, int FD) {



		Client ClientToAdd;



		strcpy(ClientToAdd.IPaddress,ip);



		strcpy(ClientToAdd.Name,Name);



		ClientToAdd.ListeningPort=LP;



		ClientToAdd.FD=FD;

		

		ClientToAdd.MessagesReceived=0;

		

		ClientToAdd.MessagesSent=0;

		

		ClientToAdd.LoggedIn=1;

		for (int i=0; i<5; i++){



			int N=List[i].FD;



			if (N == -2){



				List[i]=ClientToAdd;



				return (0);

			}

			if (N==-1){

				List[i].ListeningPort==LP;

				List[i].LoggedIn==1;

			}



	}

	}

int GetClientByIP(char* IP){

	printf("GET CLIENT BY IP CALLED with IP address of *%s*\n",IP);

	char* ListIP=malloc(30*sizeof(char));

	for (int i=0;i<5; i++){

		printf("About to access list[i].IPaddress\n");

		printf("List[i].IPaddress is *%s*\n",List[i].IPaddress);

		strcpy(ListIP,List[i].IPaddress);

		if (strcmp(ListIP,IP)==0){

			printf("GET CLIENT SUCCEEDED\n");

			free(ListIP);

			return i;

	}

}

	printf("GET CLIENT FAILED");

	return -1;



	}



int BlockClient(char* SourceIP,char *DestIP,int SourceSock,int DestSock){

	printf("BLOCK CLIENT CALLED\n");

	int success=0;

	for(int i=0;i<5;i++){

		Client CurrentClient=List[i];

		char* ClientToBlock=GetIPAddress(DestSock);

		if (((strcmp(CurrentClient.IPaddress,ClientToBlock)==0)&&(strcmp(CurrentClient.IPaddress,SourceIP)!=0))&&(CurrentClient.LoggedIn==1)){

			printf("IP IS VALID\n");

			//Means IP exists, IP is logged in, and IP is not equal to the Blocker IPaddress

		//checking if already blocked

			for (int j=0; j<5;j++){

				printf("CHECKING TO SEE IF CLIENT IS ALREADY BLOCKED\n");

				char* BlockedClientIP=malloc(30*sizeof(char));

				if (strcmp(BlockedClientIP,DestIP)==0){

					//client already blocked

					return 0;

				}

				else{

					printf("TRYING TO BLOCK CLIENT\n");

					int BlockerID=GetClientByIP(SourceIP);

					if (BlockerID>-1){

						printf("BLOCKER ID>-1\n");

						Client Blocker=List[BlockerID];

						printf("BLOCKER MADE\n");

						int numBlocked=Blocker.NumberOfBlocked;

						printf("BLOCKER ID IS *%d* AND NUMBEROFBLOCKED IS *%d*\n",BlockerID,numBlocked);

						printf("NumBLOCKED MADE \n");

						printf("List[BlockerID].BlockList is *%s\n*",List[BlockerID]);

						fflush(stdout);

/*						printf("List[BlockerID].BlockList[numBlocked] is *%s*\n",List[BlockerID].BlockList[numBlocked]);*/

						strcpy(Blocker.BlockList[numBlocked],ClientToBlock);

						printf("KSDJFISDFJ\n");

						List[BlockerID].NumberOfBlocked+=1;

					//Block the client

					return 1;

				}

	}

	

}

}

}

return 0;

}





































	int Create_Server(int PortNO){



		  	int port = PortNO; 



		if (port <= 0) {



			  return 1;



		}



		  	if (initialize_server(port) < 0) {



			return 1;



			  }



		else{



			PORT=port;



			server_loop();



		}



		return 0;  



		}







void SendMessage(char *Command,char *Arg1,char *Arg2,char *SenderIP,char *DataReceived, int sock_index){

	int Exists=0;

	Parse1(&Command,&Arg1,&Arg2,DataReceived);

	Arg1[strlen(Arg1)-1]='\0';

	if (strcmp(Command,"SEND")==0){

		for (int i = 0; i < 5; i++) {

			Client currentClient = List[i];

				char *ClientIP=malloc(256*sizeof(char));

				strcpy(ClientIP,currentClient.IPaddress);

				if (strcmp(Arg1,ClientIP)==0){

					 	if(List[i].FD!=sock_index){

					 		printf("List[i].FD is *%d* and Socket is *%d*\n",List[i].FD,sock_index);

							printf("Client FD is not -1\n");

							Exists=1;

							char* MessageToSender=(char*) malloc(1024*sizeof(char));

							strcpy(MessageToSender,MessageCreator(DataReceived,Command,GetIPAddress(sock_index),SenderIP,1));

							int MSLen=strlen(MessageToSender);

							send(sock_index,MessageToSender,MSLen,0);

							int NumberDestHasBlocked=currentClient.NumberOfBlocked;

							int blocked=0;

							for (int i=0;i<NumberDestHasBlocked;i++){

								if(strcmp(currentClient.BlockList[i],SenderIP)==0){

									blocked=1;

								}

							}

							if (blocked==0){

								if (currentClient.LoggedIn==1){

									char* MessageToDest=(char*) malloc(1024*sizeof(char));

									strcpy(MessageToDest,MessageCreator(Arg2,"RELAYED",GetIPAddress(sock_index),Arg1,1));

									int MDLen=strlen(MessageToDest);

									send(currentClient.FD,MessageToDest,MDLen,0);

								}

								else{

									printf("Client is not logged inSEND\n");

									send(currentClient.FD,"12",2,0);

		/*															AddToBacklog(GetIPAddress(sock_index),ClientIP,Arg2);*/

								}

								break;

							}

						}

						

				}

			}

		printf("FUCK ME\n");

		if (Exists==0){

					printf("GO FUCK YOURSELF\n");

					char* MessageToSender=(char*)malloc(1024*sizeof(char));

					strcpy(MessageToSender,MessageCreator(Command,Command,Command,Command,0));

					int MSLen=strlen(MessageToSender);

					send(sock_index,MessageToSender,MSLen,0);

		}

}

}



void BroadcastMessage(char *Command,char *Arg1,char *Arg2,char *SenderIP,char *DataReceived, int sock_index){

	int Exists=0;

		Parse1(&Command,&Arg1,&Arg2,DataReceived);

		Arg1[strlen(Arg1)-1]='\0';

		for (int i = 0; i < 5; i++) {

			Client currentClient = List[i];

			char *ClientIP=malloc(256*sizeof(char));

			strcpy(ClientIP,currentClient.IPaddress);

		 	if((List[i].FD!=sock_index)&& (List[i].FD!=-2)){

		 		printf("List[i].FD is *%d* and Socket is *%d*\n",List[i].FD,sock_index);

				printf("Client FD is not -1\n");

				Exists=1;

				if (currentClient.LoggedIn==1){

					char* MessageToDest=(char*) malloc(1024*sizeof(char));

					strcpy(MessageToDest,MessageCreator(Arg1,"RELAYED",GetIPAddress(sock_index),ClientIP,1));

					int MDLen=strlen(MessageToDest);

					send(currentClient.FD,MessageToDest,MDLen,0);

				}

				else{

					printf("Client is not logged inBROADCAST\n");

/*															AddToBacklog(GetIPAddress(sock_index),ClientIP,Arg2);*/

				}

			}

			}

			if (Exists==0){

						printf("No Clietns currently logged in\n");

						char* MessageToSender=(char*)malloc(1024*sizeof(char));

						strcpy(MessageToSender,MessageCreator(Command,Command,Command,Command,0));

						int MSLen=strlen(MessageToSender);

						send(sock_index,MessageToSender,MSLen,0);

			}

			else{

				char* MessageToSender=(char*) malloc(1024*sizeof(char));

				strcpy(MessageToSender,MessageCreator(DataReceived,Command,GetIPAddress(sock_index),SenderIP,1));

				int MSLen=strlen(MessageToSender);

				send(sock_index,MessageToSender,MSLen,0);

			}



}

	// Initialize the server



	int initialize_server(int port) {



	/*Backlog EmptyLog;*/

	EmptyLog = (Backlog*)malloc(sizeof(Backlog));



	/*Message Empty;*/

	Empty = (Message*)malloc(sizeof(Message));



	/*Client Dummy;*/

		Dummy = (Client*)malloc(sizeof(Client));

		Dummy->FD=-2;

		strcpy(Dummy->Name,"EMPTY");

		Dummy->ListeningPort=0;

		Dummy->MessagesReceived=0;

		Dummy->MessagesSent=0;

		Dummy->LoggedIn=0;

		for(int i=0;i<5;i++){

			strcpy(Dummy.BlockList[i],"");

		}

		strcpy(Dummy->IPaddress,"69");

		strcpy(Empty->SourceIP,"69");

		strcpy(EmptyLog->DestIP,"69");

		for(int i=0;i<5;i++){

			ListOfBacklogs[i]=*EmptyLog;

		}

		for (int i=0; i<5;i++){

			List[i]=*Dummy;

		}





		  PORT = port;



		  server_socket = socket(AF_INET, SOCK_STREAM, 0);



		  if (server_socket < 0) {



		      perror("Socket creation failed");



		      return -1;



		  }



		  server_addr.sin_family = AF_INET;



		  server_addr.sin_addr.s_addr = INADDR_ANY;



		  server_addr.sin_port = htons(PORT);



		  if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {



		      return -1;



		  }



		  if (listen(server_socket, 5) < 0) {  // 10 is the max number of waiting connections



		      perror("Listen failed\n");



		      return -1;



		  }



		  FD_ZERO(&master_list);



		  FD_SET(server_socket, &master_list);



		  FD_SET(0,&master_list);



		  head_socket = server_socket;



		  return 0;  // success



	}



	// Accept a new connection



	int accept_new_connection() {



		  int new_socket;



		  socklen_t addr_len = sizeof(client_addr);



		  new_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);



		  if (new_socket < 0) {



		      perror("Accept failed\n");



		      return -1;



		  }



		  // Add to master_list



		  FD_SET(new_socket, &master_list);



		  if (new_socket > head_socket){



		  	head_socket = new_socket;



		  }



		  return new_socket;



	}



	void server_loop() { 



		while (1) {



			memcpy(&watch_list, &master_list, sizeof(master_list));



			int STDIN= fileno(stdin);



			selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);



			if(selret < 0){



				perror("select failed.\n");



				}



			if(selret > 0){



				/* Loop through socket descriptors to check which ones are ready */



				for(sock_index=0; sock_index<=head_socket; sock_index+=1){



					if(FD_ISSET(sock_index, &watch_list)){



						if (sock_index == STDIN){

						

							char *cmd = (char*) malloc(sizeof(char)*CMD_SIZE);



							if(fgets(cmd, CMD_SIZE-1, stdin) == NULL){ //Mind the newline character that will be written to cmd



								exit(-1);



							}

							cmd[strlen(cmd)-1]='\0';



							if (strcmp(cmd,"EXIT")==0){



								remove_connection(head_socket);



								cse4589_print_and_log("[EXIT:SUCCESS]\n");



								cse4589_print_and_log("[EXIT:END]\n");



								exit(-1);



							}

							else if (strcmp(cmd,"AUTHOR")==0){



								handle_author_command();



							}



							else if (strcmp(cmd,"IP")==0){

							

								handle_ip_command();

							}



							else if (strcmp(cmd,"PORT")==0){



								handle_port_command(PORT);



							}

							else if (strcmp(cmd,"LIST")==0){



								char *DataToSend= ReturnMessage(List);

								cse4589_print_and_log("[LIST:SUCCESS]\n");

								cse4589_print_and_log("%s", DataToSend);

								cse4589_print_and_log("[LIST:END]\n");



							}

							else{





								cse4589_print_and_log("[%s:ERROR]\n",cmd);

								cse4589_print_and_log("[%s:END]\n",cmd);

								



								}



							free(cmd);





						}



						

						//new client trying to login

						else if(sock_index == server_socket){



							caddr_len = sizeof(client_addr);



							fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);



							if(fdaccept < 0){



								perror("Accept failed.");



							}



							char client_ip[INET_ADDRSTRLEN];



							char client_hostname[256];



							getnameinfo((struct sockaddr *)&client_addr, sizeof(client_addr), client_hostname, sizeof(client_hostname), NULL, 0, 0);



							inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));



							/* Add to watched socket list */



							FD_SET(fdaccept, &master_list);



							if(fdaccept > head_socket) {



								head_socket = fdaccept;



							}



							char *DataR = (char*) malloc(sizeof(char)*256);



							int bytes_received = recv(fdaccept, DataR, 255,0);



							if (bytes_received > 0) {



								DataR[bytes_received] = '\0';



							}

							AddClient(client_ip,client_hostname,atoi(DataR),fdaccept);



							qsort(List, 5, sizeof(Client), compareClients);



							char *DataToSend= ReturnMessage(List);



					



							send(fdaccept,DataToSend,strlen(DataToSend),0);

							//also send every message if there are messages



						}



					/* Read from existing clients */



						else{

							

							int *SOCKET = &sock_index;

							int Sock=*SOCKET;

							char *DataReceived= (char*) malloc(sizeof(char)*1023);

							

							if(recv(sock_index, DataReceived, 256, 0) <= 0){



								remove_connection(sock_index);

								printf("HOSUE");

							}



							else {

								

								printf("RECEIVED DATA FROM CLIENT:%s\n",DataReceived);

								

					

								

								if (strcmp(DataReceived,"LOGOUT")==0){

									LogClientOut(sock_index);

								}

								else if ((strcmp(DataReceived,"REFRESH")==0) || (strcmp(DataReceived,"LIST")==0)){



									char *DataToSend= ReturnMessage(List);



									send(sock_index,DataToSend,strlen(DataToSend),0);





								}

								else{

									char *Command= (char*) malloc(256*sizeof(char));

									char *Arg1= (char*) malloc(256*sizeof(char));

									char *Arg2 = (char*) malloc(256*sizeof(char));

									char *SenderIP=(char*) malloc(256*sizeof(char));

									int Exists=0;

									Parse1(&Command,&Arg1,&Arg2,DataReceived);

									strcpy(SenderIP,GetIPAddress(sock_index));

									if (strcmp(Command,"SEND")==0){

										SendMessage(Command,Arg1,Arg2,SenderIP,DataReceived,sock_index);

									}

									else if (strcmp(Command,"BROADCAST")==0){

										BroadcastMessage(Command,Arg1,Arg2,SenderIP,DataReceived,sock_index);

									}

									else if (strcmp(Command,"BLOCK")==0){

										int BlockedID=GetClientByIP(Arg1);

										int BlockSock=List[BlockedID].FD;

										int status=BlockClient(SenderIP,Arg1,sock_index,BlockSock);

										printf("TRYING TO BLOCK Arg1:*%s*");

									}

							

				



						}



					}



				}



			}

		}

	}

	}

	}
