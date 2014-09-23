#include "djb/buffer.h"

#undef msg
#define msg(...)  err_b(buffer_1,__VA_ARGS__,0)

#define BUFFER_1_LEN	3200
buffer __b_1 = BUFFER_INIT(write, 1, 0, BUFFER_1_LEN);
buffer *buffer_1 = &__b_1;

static void outs(const char *X) { buffer_puts(buffer_1, X); }
static void outc(char X) { buffer_putc(buffer_1, X); }

static void fmt_color(char *src,char color,char *suffix) {
  char ops[] =   "[1;39m"; ops[5]=color;
  outs(ops); ops[2]='0';     ops[5]='9'; 
  outs(src);
  outs(ops); 
  outs(suffix);
}
