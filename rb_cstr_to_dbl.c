#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

/* 2^64 = 20 digits */

#define DBL_DIG 15

static inline int rb_isspace(int c){ return c == ' ' || ('\t' <= c && c <= '\r'); }
static inline int rb_isdigit(int c){ return '0' <= c && c <= '9'; }
#define ISSPACE(c) rb_isspace(c)
#define ISDIGIT(c) rb_isdigit(c)

double
rb_cstr_to_dbl(const char *p, int badcheck)
{
    const char *q;
    char *end;
    double d;
    const char *ellipsis = "";
    int w;
    enum {max_width = 20};
#define OutOfRange() ((end - p > max_width) ? \
                      (w = max_width, ellipsis = "...") : \
                      (w = (int)(end - p), ellipsis = ""))

    if (!p) return 0.0;
    q = p;
    while (ISSPACE(*p)) p++;

    if (!badcheck && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        return 0.0;
    }

    d = strtod(p, &end);
    if (errno == ERANGE) {
        OutOfRange();
//        rb_warning("Float %.*s%s out of range", w, p, ellipsis);
        printf("Float %.*s%s out of range", w, p, ellipsis);
        errno = 0;
    }
    if (p == end) {
        if (badcheck) {
          bad:
//            rb_invalid_str(q, "Float()");
            printf("Float()");
        }
        return d;
    }
    if (*end) {
        char buf[DBL_DIG * 4 + 10];
        char *n = buf;
        char *e = buf + sizeof(buf) - 1;
        char prev = 0;

/* Look at p and end again after strtod()
   Look at happy path to see where end is.
   Looks like there is an extra check at end for errno
 */
        while (p < end && n < e) prev = *n++ = *p++;
/* if n == e then
   we've filled buf with as many digits as we can
   The string up to sizeof(buf)-1 will be stripped of underscores
   But chars after sizeof(buf)-1 aren't checked to see if they are digits
   buf = 69 digits + \0
*/
        while (*p) {
            if (*p == '_') {
                /* remove underscores between digits */
                if (badcheck) {
                    if (n == buf || !ISDIGIT(prev)) goto bad;
                    ++p;
                    if (!ISDIGIT(*p)) goto bad;
                }
                else {
                    while (*++p == '_');
                    continue;
                }
            }
            else {
              if (badcheck && !ISDIGIT(*p)) goto bad;
            }
            prev = *p++;
            if (n < e) *n++ = prev;
        }
        *n = '\0';
        p = buf;

        if (!badcheck && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
            return 0.0;
        }

        d = strtod(p, &end);
        if (errno == ERANGE) {
            OutOfRange();
//            rb_warning("Float %.*s%s out of range", w, p, ellipsis);
            printf("Float %.*s%s out of range", w, p, ellipsis);
            errno = 0;
        }
        if (badcheck) {
            if (!end || p == end) goto bad;
            while (*end && ISSPACE(*end)) end++;
            if (*end) goto bad;
        }
    }
    if (errno == ERANGE) {
        errno = 0;
        OutOfRange();
//        rb_raise(rb_eArgError, "Float %.*s%s out of range", w, q, ellipsis);
        printf("Float %.*s%s out of range", w, q, ellipsis);
    }
    return d;
}

int main()
{
//  char *p = "1234567890";
//  char *p = "1234567890123456789012345678901234567890123456789012345678901234567890123456z";
//                    10        20        30        40        50        60        70
  char *p = "123456789012345678901234567890123456789012345678901234567890123456789_0_1_23456";
  char *end;
  double d;

  d = rb_cstr_to_dbl(p, 1);
  printf("Double: %e\n", d);
}
