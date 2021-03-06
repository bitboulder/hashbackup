#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "dbt.h"
#include "cmd.h"
#include "mc.h"

void error(char quit,const char *fmt,...){
	va_list ap;
	fprintf(stderr,quit?"ERROR: ":"WARN: ");
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
	fprintf(stderr,"\n");
	if(quit) exit(1);
}

void usage(const char *prg){
	printf("Usage: %s COMMAND ARGUMENTS\n",prg);
	printf("       init    BASEDIR [EXPATTERN{|EXPATTERN}]\n");
	printf("       tlist   [TIMESPEC]\n");
	printf("       flist   [TIMESPEC]\n");
	printf("       flistx  [TIMESPEC] (list ext2 blocks)\n");
	printf("       diff    [-c] [TIMESPEC]\n");
	printf("       diffx   [-c] [TIMESPEC] (diff sha)\n");
	printf("       commit  [-c]\n");
	printf("       restore [FILESPEC] [TIMESPEC] [DSTDIR]\n");
	printf("       del     TIMESPEC\n");
	printf("       mctest  FILENAME\n");
	printf("       dbcheck\n");
	exit(1);
}

int main(int argc,char **argv){
	char *cmd;
	mc_init();
	dbload();
	if(argc<2) usage(argv[0]);
	cmd=argv[1];
	if(!strncmp(cmd,"init",4)){
		if(argc<3) usage(argv[0]);
		init(argv[2],argc<4?NULL:argv[3]);
	}
	else if(!strncmp(cmd,"tlist",5)){ tlist(argc<3?NULL:argv[2]); }
	else if(!strncmp(cmd,"flistx",6)){ flist(argc<3?NULL:argv[2],1); }
	else if(!strncmp(cmd,"flist",5)){ flist(argc<3?NULL:argv[2],0); }
	else if(!strncmp(cmd,"diffx",5)){ if(argc>=3 && !strncmp(argv[2],"-c",2)) diff(argc<4?NULL:argv[3],1,1); else diff(argc<3?NULL:argv[2],0,1); }
	else if(!strncmp(cmd,"diff", 4)){ if(argc>=3 && !strncmp(argv[2],"-c",2)) diff(argc<4?NULL:argv[3],1,0); else diff(argc<3?NULL:argv[2],0,0); }
	else if(!strncmp(cmd,"commit",6)){ commit(argc>=3 && !strncmp(argv[2],"-c",2)); }
	else if(!strncmp(cmd,"restore",7)){
		restore(argc<3?NULL:argv[2],argc<4?NULL:argv[3],argc<5?NULL:argv[4]);
	}
	else if(!strncmp(cmd,"del",3)){
		if(argc<3) usage(argv[0]);
		del(argv[2]);
	}
	else if(!strncmp(cmd,"dbcheck",7)){ dbcheck(); }
	else if(!strncmp(cmd,"mctest",7)){ if(argc>=3) printf("mc: %s\n",mc_get(argv[2])); }
	else usage(argv[0]);
	return 0;
}
