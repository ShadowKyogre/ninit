unsigned int s_len(const char * s) {
  unsigned int len=0;
  while (s[len]) ++len;
  return len;
}

char *fmt_ul(unsigned long u) {
  static char strnum[48];
  char *hex = "0123456789abcdef";
  char *s = strnum+44;
  *s = 0;
  do { *--s = hex[(u % 16)]; u /= 16; } while(u); /* handles u==0 */
  *--s = 'x';
  *--s = '0';
  return s;
}

char *fmt_o(unsigned char c) {
  static char x[8];
  x[4] = 0;
  x[3] = '0' + (c & 7); c >>= 3;
  x[2] = '0' + (c & 7); c >>= 3;
  x[1] = '0' + (c & 7);  
  x[0] = '\\';
  return x;
}

void w(char *s) { write(1,s,s_len(s)); }
void wn(unsigned long u) { w(fmt_ul(u)); }
