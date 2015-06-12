#ifndef _DBHWRK_H
#define _DBHWRK_H

size_t dbhsave(const unsigned char *sha,const char *fn);
size_t dbhsavebuf(const unsigned char *sha,const unsigned char *buf,size_t l);
void dbhrestore(const unsigned char *sha,const char *fn);
void dbhrestorebuf(const unsigned char *sha,unsigned char *buf,size_t l);
void dbhdel(const unsigned char *sha);

#endif
