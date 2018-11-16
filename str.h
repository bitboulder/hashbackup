#ifndef _STR_H
#define _STR_H

#define FNFLEN	1024

#include <stdint.h>

struct str {
	uint32_t l;
	char *s;
};
#define STRDEF {0,NULL}
#define STR(charbuf)	(struct str){strlen(charbuf)+1,(char*)charbuf}

struct str *str_alloc(uint32_t l);
void str_free(struct str *s);
void str_setlen(struct str *s,uint32_t l);
void str_clip(struct str *s);
void str_copy(struct str *dst,const struct str src,char clip);

#endif
