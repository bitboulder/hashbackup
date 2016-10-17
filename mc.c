#include <stdio.h>
#include <string.h>
#include <magic.h>

#include "mc.h"
#include "main.h"

static magic_t mc=NULL;

void mc_init(){
	mc=magic_open(MAGIC_MIME_TYPE);
	if(!mc) error(1,"magic library open failed\n");
	if(magic_load(mc,NULL)) error(1,"magic library load failed\n");
}

char mc_nozip(const char *fn){
	const char *mt;
	if(!mc) return 0;
	mt=magic_file(mc,fn);
	if(strstr(mt,"jpeg")) return 1;
	if(strstr(mt,"ogg")) return 1;
	if(strstr(mt,"mpeg")) return 1;
	if(strstr(mt,"video")) return 1;
	return 0;
}

const char *mc_get(const char *fn){
	if(!mc) return "ERR: magic lib not loaded";
	return magic_file(mc,fn);
}
