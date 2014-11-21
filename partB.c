#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "unistd.h"
#include <pthread.h>

FILE* db;	//"Database" file

typedef struct account {

	char accNum[6];
	char PIN[4];
	float balance;

} ACCOUNT;

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

void *atmInterface() {

	char accountNumber[56];
	char pin[4];
	int attempts = 0;
	int correct = 0;

	while(1) {

		printf("Please enter your 5 digit account number\n");
		scanf("%s", accountNumber);

		while(correct < 1 && attempts < 3) {

			printf("Please enter your 3 digit PIN\n");
			scanf("%s", pin);

			//Send message to DB server
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

int main() {

	db = fopen("db.txt","w");

	//Database initialization

	fprintf(db, "00001 107 3443.22\n00011 323 10089.97\n00117 259 112.00");
	fclose(db);

	ACCOUNT* accounts = readDB();
	printDB(accounts, 3);
	
	return 0;
}
