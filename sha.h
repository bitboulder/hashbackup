#ifndef _SHA_H
#define _SHA_H

#include "str.h"

void shaget(const char *fn,unsigned char *sha);
void shagetdb(const char *fn,unsigned char *sha);
void shastr(const char *str,unsigned char *sha);
void shabuf(const unsigned char *buf,size_t l,unsigned char *sha);
void fn2sha(const char *fn,unsigned char *sha);
void sha2fn(const unsigned char *sha,struct str *fn);

#endif
