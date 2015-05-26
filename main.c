#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "main.h"
#include "db.h"

void error(const char *fmt,...){
	va_list ap;
	fprintf(stderr,"ERROR: ");
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	fprintf(stderr,"\n");
	exit(1);
}

int main(int argc,char **argv){
	if(argc<2) error("Usage: %s DBFILE",argv[0]);
	dbload(argv[1]);
	return 0;
}
