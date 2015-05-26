#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "db.h"
#include "main.h"

#define HNCH	8192

#define FNLEN	1024

struct dbf {
	struct dbf *nxt;
	char fn[FNLEN];
};

struct dbt {
	struct dbt *nxt;
	time_t t;
	struct dbf *fhsh[HNCH];
};

struct dbt *db=NULL;

void dbload(const char *fn){
	FILE *fd;
	time_t t;
	if(!(fd=fopen(fn,"rb"))) return;
	while(fread(&t,sizeof(time_t),1,fd)==1 && t){

	}
	fclose(fd);
}

void dbsave(const char *fn){
	FILE *fd;
	if(!(fd=fopen(fn,"wb"))) error("db open failed for '%s'",fn);
	fclose(fd);
}
