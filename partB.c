#include "stdlib.h"
#include "stdio.h"
#include "string.h"

FILE* db;	//"Database" file

typedef struct account {

	int accNum;
	int PIN;
	float balance;

} ACCOUNT;

int main() {

	db = fopen("db.txt","w");

	//Database initialization

	fprintf(db, "00001,107,3443.22\n00011,323,10089.97\n00117,259,112.00\n");
	
	return 0;
}
