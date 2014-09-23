#include <unistd.h>
int main() {
  if (getpagesize() == 4096) return 0;
  return 1;
}
