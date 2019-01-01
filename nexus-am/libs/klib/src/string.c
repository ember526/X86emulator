#include "klib.h"

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
    const char *tmp;
    for (tmp = s; *tmp; ++tmp) {}
    return (tmp - s);
}

char *strcpy(char* dst,const char* src) {
   char *save = dst;
   while((*dst++ = *src++));
   return save;
}

char* strncpy(char* dst, const char* src, size_t n) {
	char* temp = dst;
	size_t i = 0;
  	assert((dst != NULL) && (src != NULL));
 	while(i++ < n && (*temp++ = *src++)!='\0');
	return dst;
}

char* strcat(char* dst, const char* src) {
	strcpy(dst + strlen(dst), src);
    return dst;
}

int strcmp(const char* s1, const char* s2) {
	unsigned char u1, u2;
  	while (1)
  	{
  	  	u1 = (unsigned char) *s1++;
  	  	u2 = (unsigned char) *s2++;
  	  	if (u1 != u2)
			return u1 - u2;
  	  	if (u1 == '\0')
			return 0;
  	}
  	return 0;
}

int strncmp(const char* s1, const char* s2, size_t n) {
	unsigned char u1, u2;
  	while (n-- > 0)
  	  {
  	    u1 = (unsigned char) *s1++;
  	    u2 = (unsigned char) *s2++;
  	    if (u1 != u2)
			return u1 - u2;
  	    if (u1 == '\0')
			return 0;
  	  }
  	return 0;
}

void* memset(void* v,int c,size_t n) {
	unsigned char *ptr = (unsigned char*)v;
  	while (n-- > 0)
    	*ptr++ = c;
  	return v;
}

void* memcpy(void* out, const void* in, size_t n) 
{
  if (out < in)
    {
      const char *firsts = (const char *) in;
      char *firstd = (char *) out;
      while (n--)
	 *firstd++ = *firsts++;
    }
  else
    {
      const char *lasts = (const char *)in + (n-1);
      char *lastd = (char *)out + (n-1);
      while (n--)
        *lastd-- = *lasts--;
    }
    return out;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    while (n-- > 0)
    {
      if (*(unsigned char *)s1++ != *(unsigned char *)s2++)
	  	return ((unsigned char *)s1)[-1] < ((unsigned char *)s2)[-1] ? -1 : 1;
    }
  	return 0;
}

#endif
