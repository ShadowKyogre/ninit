#define SCAN_NUMBER_DEFINE(name, type, base) \
unsigned int name(const char *s, type *u) {\
  unsigned int pos;\
  type result, c;\
  pos = 0; result = 0;\
  while ((c = (type) (unsigned char) (s[pos] - '0')) < base)\
    { result = result * base + c; ++pos; }\
  *u = result; return pos;\
}
