#ifndef _SHA_H
#define _SHA_H

void shaget(const char *fn,unsigned char *sha);
void shagetdb(const char *fn,unsigned char *sha);
void shastr(const char *str,unsigned char *sha);
void fn2sha(const char *fn,unsigned char *sha);
void sha2fn(const unsigned char *sha,char *fn);

#endif
