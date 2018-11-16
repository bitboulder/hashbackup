#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "main.h"

struct str *str_alloc(uint32_t l){
	struct str *s=malloc(sizeof(struct str));
	str_setlen(s,l);
	return s;
}

void str_free(struct str *s){
	str_setlen(s,0);
	free(s);
}

void str_setlen(struct str *s,uint32_t l){
	s->l=l;
	if(s->s!=NULL){
		if(s->l) s->s=realloc(s->s,s->l);
		else{ free(s->s); s->s=NULL; }
	}else if(s->l){
		s->s=malloc(l);
		s->s[0]=0;
	}
}

void str_clip(struct str *s){ str_setlen(s,strlen(s->s)+1); }

void str_copy(struct str *dst,const struct str src,char clip){
	uint32_t l=strlen(src.s)+1;
	if(clip) str_setlen(dst,l);
	else if(dst->l<l) error(1,"not enought space in dst");
	memcpy(dst->s,src.s,l);
}

