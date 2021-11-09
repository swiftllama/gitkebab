#include <iconv.h>

extern size_t libiconv (iconv_t cd, char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft);
extern iconv_t libiconv_open (const char* tocode, const char* fromcode);
extern int libiconv_close (iconv_t cd);
