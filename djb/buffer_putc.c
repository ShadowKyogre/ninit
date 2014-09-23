#include "buffer.h"

int buffer_putc(buffer *b, char ch) /*EXTRACT_INCL*/ {
  return buffer_put(b, &ch, 1);
}
