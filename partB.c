#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

FILE* db;	//"Database" file

typedef struct account {

	char* accNum;
	char* PIN;
	char* balance;

} ACCOUNT;

void updateDB(FILE* db, ACCOUNT* accounts, int numAccounts) {

	db = fopen("db.txt","w");

	for(int i = 0; i < numAccounts; i++) {

		fprintf(db, "%s,%s,%s",accounts[i].accNum,accounts[i].PIN,accounts[i].balance);

	}

	fclose(db);

}

ACCOUNT* readDB(FILE* db) {

	ACCOUNT* arr;
	char* accbuf;
	char* pinbuf;
	char* balbuf;
	int inc;
	int counter = 0;

	while(fscanf(db, "%s,%s,%s",accbuf,pinbuf,balbuf) != EOF) {

		ACCOUNT* temp = (ACCOUNT*)malloc(sizeof(ACCOUNT));
		assert(temp);
		strcpy(temp->accNum,accbuf);
		strcpy(temp->PIN,pinbuf);
		strcpy(temp->balance, balbuf);

		ACCOUNT* t = (ACCOUNT*)realloc(arr, sizeof(ACCOUNT)*(counter+1));

		if(t == NULL) {
			printf("NULL\n");
		}
		else {
			printf("SWEET\n");
			arr = t;
		}

		counter++;

	}

	return arr;

}

void printDB(ACCOUNT* arr, int length) {

	for(int i = 0; i < length; i++) {

		printf("%s %s %s\n", arr[i].accNum, arr[i].PIN, arr[i].balance);

	}

}

int main() {

	db = fopen("db.txt","w");

	//Database initialization

	fprintf(db, "00001,107,3443.22\n00011,323,10089.97\n00117,259,112.00\n");

	ACCOUNT* accounts = readDB(db);
	if(accounts == NULL) { printf("FUCK\n"); }
	printDB(accounts, 3);
	
	return 0;
}
