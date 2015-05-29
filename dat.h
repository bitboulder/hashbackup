#ifndef _DAT_H
#define _DAT_H

void mkd(const char *fn);
size_t datadd(const unsigned char *sha,const char *fn);
void datget(const unsigned char *sha,const char *fn);
void datdel(const unsigned char *sha);
size_t datsi(const unsigned char *sha);

#endif
