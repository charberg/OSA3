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

ACCOUNT* accounts; //Shared database
int accCount = 0;	//Number of accounts in database

typedef struct message {

	/****MESSAGE TYPES


	1: AccountNumber + PIN in mtext
	2: Y/N in mtext
	3: AccountNumber to be locked
	4: Balance requested/sent back
	5: AccountNumberPINWithdrawal, to update database after withdrawal


	*****************/

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
	accCount = 1;

	while(!feof(db)) {
		ACCOUNT* temp = (ACCOUNT*)malloc(sizeof(ACCOUNT));
		fscanf(db, "%s %s %f",temp->accNum, temp->PIN, &temp->balance);
		assert(temp);
		ACCOUNT* t = (ACCOUNT*)realloc(arr, sizeof(ACCOUNT)*(accCount+1));
		
		if(t != NULL) {
			arr = t;
		}
		
		arr[accCount] = *temp;

		accCount++;
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

	char accountNumber[6];
	char pin[4];
	int attempts = 0;
	int correct = 0;

	int msqid;
	int msrid;
	int msgflg = IPC_CREAT | 0666;
	key_t key;
	key_t inKey;
	MESSAGE mail;

	while(1) {

		printf("Please enter your 5 digit account number\n");
		scanf("%s", accountNumber);

		while(correct < 1 && attempts < 3) {

			printf("Please enter your 3 digit PIN\n");
			scanf("%s", pin);

			strcpy(mail.mtext,accountNumber);
			strcat(mail.mtext,pin);
			mail.mtype = 1;	//1 means Account Number AND PIN

			key = 1111;
			
			if((msqid = msgget(key, msgflg)) < 0) {
				printf("ERROR 1\n");
				perror("msgget");
				exit(1);
			}

			inKey = 3333;
			
			if((msrid = msgget(inKey, msgflg)) < 0) {
				printf("ERROR 1\n");
				perror("msgget");
				exit(1);
			}


			if(msgsnd(msqid, &mail, strlen(mail.mtext) + 1, 0) < 0) {

				printf("ERROR 2\n");
				perror("msgsnd");
				exit(1);
			}	

			printf("Sent\n");

			//At this point, account number has been sent to other thread

			//Send message to DB server
			//if(messageBack -> pin == FALSE)
			//increment counter
			//else if (messageBack -> pin == TRUE)
			// correct = TRUE	

			msgrcv(msrid, &mail, 100, 2, 0);

			if(strcmp(mail.mtext, "Y") == 0) {

				correct = 1;	//Means PIN and account number combination were valid

			}

			else {
				attempts++;
			}

		}

		if(attempts > 2) {

			//Send message to lock account!!

			mail.mtype = 3;	//Locking message
			strcpy(mail.mtext, accountNumber);

			if(msgsnd(msqid, &mail, strlen(mail.mtext) + 1, 0) < 0) {

				printf("ERROR 2 SENDING LOCK MESSAGE FAILURE\n");
				perror("msgsnd");
				exit(1);
			}

			printf("CTRL C TO QUIT\n");

			while(1) {

			}

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
			//Message of type 4

			mail.mtype = 4;
			strcpy(mail.mtext, accountNumber);

			if(msgsnd(msqid, &mail, strlen(mail.mtext) + 1, 0) < 0) {

				printf("ERROR 2 SENDING BALANCE REQUEST FAILURE\n");
				perror("msgsnd");
				exit(1);
			}		
			
			msgrcv(msrid, &mail, 100, 4, 0);	//Wait for balance from server

			printf("Account balance: %s\n", mail.mtext);

			break;
	
		case 2:
			//Request DB Server for withdrawal for accountNumber
			//Check if withdraw request is reasonable
			//If yes, print SUCCESS
			//If no, print NOPE

			strcpy(mail.mtext, accountNumber);
			strcat(mail.mtext, pin);
			char temp[100];

			printf("Please enter withdrawal amount\n");
			scanf("%s", temp);

			strcat(mail.mtext, temp);	//Add withdrawal amount to message string

			mail.mtype = 5;	//Withdrawal request

			if(msgsnd(msqid, &mail, strlen(mail.mtext) + 1, 0) < 0) {

				printf("ERROR 2 SENDING WITHDRAWAL REQUEST FAILURE\n");
				perror("msgsnd");
				exit(1);
			}		

			msgrcv(msrid, &mail, 100, 5, 0);	//Wait for confirmation from server
			printf("%s\n", mail.mtext);

			break;
	
		default:
			printf("Invalid option\n");
			break;

		}
	
	}

}

void * dbServer(void* a) {

	int msqid;
	int msrid;
	key_t key;
	key_t dbkey;
	key_t inKey;
	MESSAGE mail;
	int mdbid;

	key = 1111;
	dbkey = 2222;
	inKey = 3333;

	if((msqid = msgget(key, IPC_CREAT | 0666)) < 0) {
		printf("ERROR 3 HERE\n");
		perror("msgget");
		exit(1);
	}


	if((mdbid = msgget(dbkey, IPC_CREAT | 0666)) < 0) {	//Create db editor message queue
		printf("ERROR 3 SECOND\n");
		perror("msgget");
		exit(1);
	}

	if((msrid = msgget(inKey, IPC_CREAT | 0666)) < 0) {	//Create db editor message queue
		printf("ERROR 3 SECOND\n");
		perror("msgget");
		exit(1);
	}

	while(1) {

		int check = 0;
		mail.mtype = 0;

		msgrcv(msqid, &mail, 100, 1, IPC_NOWAIT);
		msgrcv(msqid, &mail, 100, 3, IPC_NOWAIT);
		msgrcv(msqid, &mail, 100, 4, IPC_NOWAIT);
		msgrcv(msqid, &mail, 100, 5, IPC_NOWAIT);

		if(mail.mtype == 1) {	//If received an initial account log in message

			char accNumber[6];
			char PIN[4];
		
			for(int i = 0; i < 5; i++) {	//Copy in the accNumber and PIN from the message

				accNumber[i] = mail.mtext[i];

			}

			for(int i = 0; i < 3; i++) {

				PIN[i] = mail.mtext[i+5];

			}

			accNumber[5] = '\0';
			PIN[3] - '\0';	//Terminate the data strings

			for(int i = 0; i < accCount; i++) {

				if(strcmp(accounts[i].accNum, accNumber) == 0 && strcmp(accounts[i].PIN, PIN) == 0) {

					check = 1;	//Confirm that account and PIN are correct	

				}

			}

			if(check == 1) {
				strcpy(mail.mtext, "Y");
			}

			else {
				strcpy(mail.mtext, "N");
			}

			mail.mtype = 2;	//Set message type to confirmation message

			if(msgsnd(msrid, &mail, strlen(mail.mtext) + 1, 0) < 0) {

				printf("ERROR 2 SEND FROM ACC PIN CHECK\n");
				perror("msgsnd");
				exit(1);
			}		

		}

		///////////////////////////////////////////////////////////////////////////////////

		if(mail.mtype == 3) {	//If account needs to be locked

			printf("SERVER RECEIVED LOCK REQUEST\n");
	
			//Simply forward message to db editor by changing to mdbid queue

			if(msgsnd(mdbid, &mail, strlen(mail.mtext) + 1, 0) < 0) {	//Send account number to editor to be locked

				printf("ERROR SENDING TO DB EDITOR\n");
				perror("msgsnd");
				exit(1);

			}

		}

		//////////////////////////////////////////////////////////////////////////////////

		if(mail.mtype == 4) {	//If balance is requested

			float foundBalance;

			for(int i = 0; i < accCount; i++) {

				if(strcmp(accounts[i].accNum, mail.mtext) == 0) {

					//If matching account has been found, return balance
					foundBalance = accounts[i].balance;

				}

			}

			snprintf(mail.mtext, 100, "%f", foundBalance);
			
	
			if(msgsnd(msrid, &mail, strlen(mail.mtext) + 1, 0) < 0) {	//Send balance to editor to user to be displayed

				printf("ERROR SENDING BALANCE TO USER\n");
				perror("msgsnd");
				exit(1);

			}

		

		}
		
		/////////////////////////////////////////////////////////////////////////////////

		if(mail.mtype == 5) {	//If withdrawal request from interface

			//Notify user whether withdrawal is allowed or not

			char accountNumber[6];
			char PIN[4];
			char withdrawalString[100];
			float withdrawal;
			float balance;

			for(int i = 0; i < 5; i++) {	//Copy in account number
				accountNumber[i] = mail.mtext[i];
			}

			for(int i = 5; i < 8; i++) {	//Copy in PIN
				PIN[i-5] = mail.mtext[i];
			}

			int i = 8;
			while(mail.mtext[i] != '\0') {	//Copy in withdrawal amount
				withdrawalString[i-8] = mail.mtext[i];
				i++;
			}

			withdrawalString[i-8] = '\0';	//Terminate string, as above loop does not

			withdrawal = strtof(withdrawalString, NULL);	//Convert withdrawal string to floar, for balance comparison

			for(int i = 0; i < accCount; i++) {

				if(strcmp(accounts[i].accNum, accountNumber) == 0) {

					balance = accounts[i].balance;	

				}

			}

			//At this point, the balance of the account has been found. Compare to withdrawal, to see if valid request

			if(balance < withdrawal) {

				//Send "NOT ENOUGH FUNDS" back to user
				strcpy(mail.mtext, "NOT ENOUGH FUNDS");

				if(msgsnd(msrid, &mail, strlen(mail.mtext) + 1, 0) < 0) {	//Send balance to editor to user to be displayed

					printf("ERROR SENDING FAILED WITHDRAWAL RESULT TO USER\n");
					perror("msgsnd");
					exit(1);

				}

			}

			else {	//If enough funds, notify user, and update database with new amount

				strcpy(mail.mtext, accountNumber);	//Copy into message the account info, with the new balance
				strcat(mail.mtext, PIN);

				char newBalance[100];
				float result = balance - withdrawal;
				snprintf(newBalance, 100, "%f", result);

				strcat(mail.mtext, newBalance);
	
				if(msgsnd(mdbid, &mail, strlen(mail.mtext) + 1, 0) < 0) {	//Send balance to to be displayed

					printf("ERROR SENDING ACCOUNT WITHDRAWAL INFO TO EDITOR\n");
					perror("msgsnd");
					exit(1);

				}

			}

		}

	}

}

void *  dbEditor(void* a) {

	key_t dbkey;
	MESSAGE mail;
	int mdbid;
	key_t inKey;
	int msrid;
	
	dbkey = 2222;
	inKey = 3333;

	if((mdbid = msgget(dbkey, IPC_CREAT | 0666)) < 0) {	//Create db editor message queue
		printf("ERROR 3 SECOND\n");
		perror("msgget");
		exit(1);
	}

	if((msrid = msgget(inKey, IPC_CREAT | 0666)) < 0) {	//Create db editor message queue
		printf("ERROR 3 SECOND\n");
		perror("msgget");
		exit(1);
	}

	while(1) {

		mail.mtype = 0;

		msgrcv(mdbid, &mail, 100, 3, IPC_NOWAIT);
		msgrcv(mdbid, &mail, 100, 5, IPC_NOWAIT);
	
		if(mail.mtype == 3) {	//If received account number to be locked

			for(int i = 0; i < accCount; i++) {

				if(strcmp(accounts[i].accNum, mail.mtext) == 0) {

					accounts[i].accNum[4] = 'X';
					printf("ACCOUNT LOCKED: %s\n", accounts[i].accNum);

				}

			}

		}

		if(mail.mtype == 5) {

			char accountNumber[6];
			char PIN[4];
			char balanceString[100];

			for(int i = 0; i < 5; i++) {	//Copy in account number
				accountNumber[i] = mail.mtext[i];
			}

			for(int i = 0; i < 3; i++) {	//Copy in PIN
				PIN[i] = mail.mtext[i+5];
			}

			int i = 8;
			while(mail.mtext[i] != '\0') {	//Copy in updated balance amount
				balanceString[i-8] = mail.mtext[i];
				i++;
			}

			balanceString[i] = '\0';	//Terminate string

			for(int i = 0; i < accCount; i++) {	//Find account, and update balance

				if(strcmp(accounts[i].accNum, accountNumber) == 0 && strcmp(accounts[i].PIN, PIN) == 0) {
					accounts[i].balance = strtof(balanceString, NULL);
				}

			}

			strcpy(mail.mtext, "SUCCESS\n");
			if(msgsnd(msrid, &mail, strlen(mail.mtext) + 1, 0) < 0) {	//Send balance confirmation to user to be displayed

				printf("ERROR SENDING SUCCESSFUL WITHDRAWAL RESULT TO USER\n");
				perror("msgsnd");
				exit(1);

			}

			updateDB(db, accounts, accCount);	//Update database text file with new information
			printDB(accounts, accCount);
		}

	}

}

int main() {

	db = fopen("db.txt","w");

	//Database initialization

	fprintf(db, "00001 107 3443.22\n00011 323 10089.97\n00117 259 112.00");
	fclose(db);

	accounts = readDB();
	printDB(accounts, 3);

	pthread_t atm;
	pthread_t server;
	pthread_t editor;

	pthread_create(&atm, NULL, atmInterface, (void*)NULL);
	pthread_create(&server, NULL, dbServer, (void*)NULL);
	pthread_create(&editor, NULL, dbEditor, (void*)NULL);

	pthread_join(atm, NULL);
	pthread_join(server, NULL);
	pthread_join(editor, NULL);
	
	return 0;
}
