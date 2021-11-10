
// For use on macOS where iconv, iconv_open, and iconv_close are
// macro'ed in by iconv's header from libiconv, libiconv_open and
// libiconv_close, but gitkebab won't see that when it is linked and
// libgit2 has symbol references to the former
#include "iconv_compat.h"

size_t iconv (iconv_t cd, char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft) {
  return libiconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
}

iconv_t iconv_open (const char* tocode, const char* fromcode){
  return libiconv_open(tocode, fromcode);
}

int iconv_close (iconv_t cd) {
  return libiconv_close(cd);
}
