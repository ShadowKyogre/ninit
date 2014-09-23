unsigned int splitmem(char **v, char *s, char c) /*EXTRACT_INCL*/ {
  if (v) {
    char **w=v;
    *w++=s;
    for (;;) {
      while (*s && *s!=c) s++;
      if (*s==0) break;
      *s=0;
      *w++ = ++s;
    }
    *w=0;
    return (w-v);
  } else {
    unsigned int n=1;
    for (; *s; s++) if (*s==c) n++;
    return n;
  }
}
