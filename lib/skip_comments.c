void skip_comments(char **s) /*EXTRACT_INCL*/ {
  char **d, *p;
  for (d=s; (p=*s); s++) {
    if (p[0] == '#') {
      if (p[1] == '#') ++p;
      else continue;
    }
    *d++ = p;
  }
  *d=0;
}
