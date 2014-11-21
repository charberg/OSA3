#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "unistd.h"

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
	printf("%s,%s,%f\n", arr->accNum, arr->PIN, arr->balance);
	int inc;
	int counter = 0;

	while(!feof(db)) {
		ACCOUNT* temp = (ACCOUNT*)malloc(sizeof(ACCOUNT));
		fscanf(db, "%s %s %f",temp->accNum, temp->PIN, &temp->balance);
		assert(temp);
		ACCOUNT* arr = (ACCOUNT*)realloc(arr, sizeof(ACCOUNT)*(counter+1));
		/*
		if(t == NULL) {
			printf("NULL\n");
		}
		else {
			printf("SWEET\n");
			arr = t;
		}
		*/
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

int main() {

	db = fopen("db.txt","w");

	//Database initialization

	fprintf(db, "00001 107 3443.22\n00011 323 10089.97\n00117 259 112.00");
	fclose(db);

	ACCOUNT* accounts = readDB();
	if(accounts == NULL) { printf("FUCK\n"); }
	printDB(accounts, 1);
	
	return 0;
}
