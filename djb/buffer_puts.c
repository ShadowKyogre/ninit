#include "buffer.h"

int buffer_puts(buffer *b, const char* s) /*EXTRACT_INCL*/ {
  return buffer_put(b, s, str_len(s));
}

