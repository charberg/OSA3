#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "unistd.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

FILE* db;	//"Database" file

typedef struct account {

	char accNum[6];
	char PIN[4];
	float balance;

} ACCOUNT;

typedef struct message {

	long mtype;
	char mtext[100];

} MESSAGE;

void updateDB(FILE* db, ACCOUNT* accounts, int numAccounts) {

	db = fopen("db.txt","w");

	for(int i = 0; i < numAccounts; i++) {

		fprintf(db, "%s,%s,%f",accounts[i].accNum,accounts[i].PIN,accounts[i].balance);

	}

	fclose(db);

}

ACCOUNT* readDB() {

	FILE* db = fopen("db.txt", "r");
	ACCOUNT* arr;
	if(!feof(db)) {
		arr = (ACCOUNT*) malloc(sizeof(ACCOUNT));
		fscanf(db, "%s %s %f", arr->accNum, arr->PIN, &arr->balance);
	}
	int inc;
	int counter = 1;

	while(!feof(db)) {
		ACCOUNT* temp = (ACCOUNT*)malloc(sizeof(ACCOUNT));
		fscanf(db, "%s %s %f",temp->accNum, temp->PIN, &temp->balance);
		assert(temp);
		ACCOUNT* t = (ACCOUNT*)realloc(arr, sizeof(ACCOUNT)*(counter+1));
		
		if(t != NULL) {
			arr = t;
		}
		
		arr[counter] = *temp;

		counter++;
	}
	fclose(db);
	return arr;

}

void printDB(ACCOUNT* arr, int length) {

	for(int i = 0; i < length; i++) {

		printf("%s %s %f\n", arr[i].accNum, arr[i].PIN, arr[i].balance);

	}

}

void * atmInterface(void* a) {

	char accountNumber[56];
	char pin[4];
	int attempts = 0;
	int correct = 0;

	int msqid;
	int msgflg = IPC_CREAT | 0666;
	key_t key;
	MESSAGE mail;

	while(1) {

		printf("Please enter your 5 digit account number\n");
		scanf("%s", accountNumber);

		while(correct < 1 && attempts < 3) {

			printf("Please enter your 3 digit PIN\n");
			scanf("%s", pin);

			attempts++;
	
			strcpy(mail.mtext,accountNumber);
			strcat(mail.mtext,pin);
			mail.mtype = 1;	//1 means Account Number AND PIN

			key = 1111;
			
			if((msqid = msgget(key, msgflg)) < 0) {
				printf("ERROR 1\n");
				perror("msgget");
				exit(1);
			}

			if(msgsnd(msqid, &mail, strlen(mail.mtext) + 1, 0) < 0) {

				printf("ERROR 2\n");
				perror("msgsnd");
				exit(1);
			}	

			//At this point, account number has been sent to other thread

			//Send message tdo DB server
			//if(messageBack -> pin == FALSE)
			//increment counter
			//else if (messageBack -> pin == TRUE)
			// correct = TRUE	

		}

		if(attempts > 2) {

			printf("ACCOUNT LOCKED\n");
			exit(0);

		}

		//At this point, the correct PIN has been entered for the account number
	
		attempts = 0;	//Re-set the number of attempts on the account
		correct = 0;	//Re-set for next customer
		int selection = 0;

		printf("Please select an option:\n1. Display funds\n2. Withdraw funds\n");
		scanf("%d", &selection);
	
		switch(selection) {

		case 1:
			//Request DB Server for funds for accountNumber
			break;
	
		case 2:
			//Request DB Server for withdrawal for accountNumber
			//Check if withdraw request is reasonable
			//If yes, print SUCCESS
			//If no, print NOPE
			break;
	
		default:
			printf("Invalid option\n");
			break;

		}
	
	}

}

void * dbServer(void* a) {

	int msqid;
	key_t key;
	MESSAGE mail;

	key = 1111;

	if((msqid = msgget(key, 0666)) < 0) {
		printf("ERROR 3\n");
		perror("msgget");
		exit(1);
	}

	while(1) {

		msgrcv(msqid, &mail, 100, 1, 0);

		printf("Message type: %ld Contents: %s", mail.mtype, mail.mtext);

	}

}

int main() {

	db = fopen("db.txt","w");

	//Database initialization

	fprintf(db, "00001 107 3443.22\n00011 323 10089.97\n00117 259 112.00");
	fclose(db);

	ACCOUNT* accounts = readDB();
	printDB(accounts, 3);

	pthread_t atm;
	pthread_t server;

	pthread_create(&atm, NULL, atmInterface, (void*)NULL);
	pthread_create(&server, NULL, dbServer, (void*)NULL);

	pthread_join(atm, NULL);
	pthread_join(server, NULL);
	
	return 0;
}
