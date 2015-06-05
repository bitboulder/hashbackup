#ifndef _DBHWRK_H
#define _DBHWRK_H

size_t dbhsave(const unsigned char *sha,const char *fn);
void dbhrestore(const unsigned char *sha,const char *fn);
void dbhdel(const unsigned char *sha);

#endif
