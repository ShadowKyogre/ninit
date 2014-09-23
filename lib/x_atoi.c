
int x_atoi(const char *src) /*EXTRACT_INCL*/ {
  register const char *s;
  register long int dest=0;
  register unsigned char c;

  s=src;
  if (*s=='-' /* || *s=='+' */) ++s;

  while ((c=*s-'0')<10) { ++s; dest=dest*10 + c; }
  return (*src=='-') ? -dest : dest;
}
