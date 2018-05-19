#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char **argv)
{
  if (argc < 2) {
    printf("Usage: %s string - converts to double\n", argv[0]);
    exit(1);
  }
//  char *p = "1234567890";
//  char *p = "123456789z0123456789012345678901234567890123456789012345678901234567890123456z";
  char *p = argv[1];
  char *end;
  double d;

  d = strtod(p, &end);
  if (errno != 0) {
    printf("Error: %d\n", errno);
  }
  if (*end) {
    printf("End: %s\n", end);
  }
  printf("P: %s\n", p);
  printf("Double: %e\n", d);
}
