int main() {
  unsigned long u=1,k=0;
  for (; k<32; k++) u += u;
  if (!u) return 0;
  return 1;
}
