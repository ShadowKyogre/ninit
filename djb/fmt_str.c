unsigned int fmt_str(char *s, const char *t) /*EXTRACT_INCL*/ {
  register unsigned int len;
  char ch;
  len = 0;
  if (s) { while ((ch = t[len])) s[len++] = ch; }
  else while (t[len]) len++;
  return len;
}
