#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "db.h"
#include "cmd.h"

void error(const char *fmt,...){
	va_list ap;
	fprintf(stderr,"ERROR: ");
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	fprintf(stderr,"\n");
	exit(1);
}

void usage(const char *prg){
	error("Usage: %s (init|diff|commit|dbcheck|tlist|flist|del)",prg);
}

int main(int argc,char **argv){
	char *cmd;
	dbload();
	if(argc<2) usage(argv[0]);
	cmd=argv[1];
	if(!strncmp(cmd,"init",3)){
		if(argc<3) error("Usage: %s init BASEDIR",argv[0]);
		init(argv[2]);
	}
	else if(!strncmp(cmd,"diff",4)){ diff(argc<3?NULL:argv[2]); }
	else if(!strncmp(cmd,"commit",4)){ commit(); }
	else if(!strncmp(cmd,"dbcheck",4)){ dbcheck(); }
	else if(!strncmp(cmd,"tlist",4)){ tlist(); }
	else if(!strncmp(cmd,"flist",4)){ flist(argc<3?NULL:argv[2]); }
	else if(!strncmp(cmd,"del",4)){
		if(argc<3) error("Usage: %s del TIMESPEC",argv[0]);
		del(argv[2]);
	}else usage(argv[0]);
	return 0;
}
