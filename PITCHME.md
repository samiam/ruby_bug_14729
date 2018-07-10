---
@title[Introduction]

# Ruby Float() Bug

#### Converting an invalid string into a float

#### Sam Napolitano

Note:

- How bug was discovered

---
@title[Float examples]

```ruby
irb(main):001:0> Float("3.14")
=> 3.14
irb(main):002:0> Float("3.14a")
ArgumentError: invalid value for Float(): "3.14a"
```

Note:

- Noticed that really long string was valid but mistyped character and it was still valid

---
@title[Bug Script Output]

```ruby
1a len=1 val=error
12a len=2 val=error
123a len=3 val=error
...
1234567890123456789012345678901234567890123456789012345678901234567a len=67 val=error
12345678901234567890123456789012345678901234567890123456789012345678a len=68 val=error
123456789012345678901234567890123456789012345678901234567890123456789a len=69 val=1.234567890123457e+68
1234567890123456789012345678901234567890123456789012345678901234567890a len=70 val=1.234567890123457e+68
```

Note:

- Wrote script to try to convert via Float() growing string
- Length does not include 'a'
- String of 69 chars + 'a' is valid
- clue that something is going on some buffer size

---
@title[strtod()]

```C
double
     strtod(const char *restrict nptr, char **restrict endptr);
```

>If endptr is not NULL, a pointer to the character after the last character used in the conversion is stored in the location referenced by endptr.

>If no conversion is performed, zero is returned and the value of nptr is stored in the location referenced by endptr.

---
@title[Forensics]

[object.c](https://bugs.ruby-lang.org/projects/ruby-trunk/repository/revisions/63130/entry/object.c#L3232)

---
@title[rb_cstr_to_dbl_raise - 1/3]

```C
static double
rb_cstr_to_dbl_raise(const char *p, int badcheck, int raise, int *error)
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
```

---
@title[rb_cstr_to_dbl_raise - 2/3]

```C
d = strtod(p, &end);
if (errno == ERANGE) {
    OutOfRange();
    rb_warning("Float %.*s%s out of range", w, p, ellipsis);
    errno = 0;
}
if (p == end) {
    if (badcheck) {
      bad:
        if (raise)
            rb_invalid_str(q, "Float()");
        else {
            if (error) *error = 1;
            return 0.0;
        }
    }
    return d;
}
```

---
@title[rb_cstr_to_dbl_raise - 3/3]

```C
if (*end) {
    char buf[DBL_DIG * 4 + 10];
    char *n = buf;
    char *e = buf + sizeof(buf) - 1;
    char prev = 0;
    while (p < end && n < e) prev = *n++ = *p++; // move to where strtod stopped
    while (*p) {
        if (*p == '_') {
            /* remove an underscore between digits */
            if (n == buf || !ISDIGIT(prev) || (++p, !ISDIGIT(*p))) {
                if (badcheck) goto bad;
                break;
            }
        }
        prev = *p++;
        if (n < e) *n++ = prev;
    }
    *n = '\0';
    p = buf;
```

Note:

- DBL_DIG is usually 15
- buf is 70
- p points to string passed in
- n points to buf
- e points to end of buf w/one space for NULL
- prev points to prev char of p
- buf is set with prev char IFF n < e

---
@title[strtod specification]

```
>>-+------------+--+-----+-------------------------------------->
   '-whitespace-'  +- + -+   
                   '- – -'   

>--+-+-digits--+---+--+--------+-+--+------------------------+-----------------+-><
   | |         '-.-'  '-digits-' |  '-+-e-+--+-----+--digits-'                 |   
   | '-.--digits-----------------'    '-E-'  +- + -+                           |   
   |                                         '- – -'                           |   
   '-0--+-x-+--+-hexdigits--+---+--+-----------+-+--+------------------------+-'   
        '-X-'  |            '-.-'  '-hexdigits-' |  '-+-p-+--+-----+--digits-'     
               '-.--hexdigits--------------------'    '-P-'  +- + -+               
                                                             '- – -'               
```
@size[20px](https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_72/rtref/strtod.htm)

---
@title[strtod examples]

```ruby
irb(main):0:0> Float("1.2345e+3")
=> 1234.5
irb(main):0:0> Float("1.2345e-3")
=> 0.0012345
irb(main):0:0> Float("  0x1p3")
=> 8.0
irb(main):0:0> Float("  0x2p3")
=> 16.0
irb(main):0:0> "%b" % Float("  0x1p3")
=> "1000"
```

---
@title[object.c on trunk]
```ruby
while (*p) {
    if (*p == '_') {
        /* remove an underscore between digits */
        if (n == buf || !ISDIGIT(prev) || (++p, !ISDIGIT(*p))) {
            if (badcheck) goto bad;
            break;
        }
    }
    prev = *p++;
    if (e == init_e && (prev == 'e' || prev == 'E' || prev == 'p' || prev == 'P')) {
        e = buf + sizeof(buf) - 1;
        *n++ = prev;
        switch (*p) {case '+': case '-': prev = *n++ = *p++;}
        if (*p == '0') {
            prev = *n++ = '0';
            while (*++p == '0');
        }
        continue;
    }
    else if (ISSPACE(prev)) {
        while (ISSPACE(*p)) ++p;
        if (*p) {
            if (badcheck) goto bad;
            break;
        }
    }
    else if (prev == '.' ? dot_seen++ : !ISDIGIT(prev)) {
        if (badcheck) goto bad;
        break;
    }
    if (n < e) *n++ = prev;
}
*n = '\0';
p = buf;
```

Note:
- Line 194 is where additional check is made to ensure chars are digits

---?code=presentation/object-v63130.c&lang=c&title=Broken Code @title[object-v63130.c]

@[3231-3232](function name) @[3252](first attempt to convert string) @[3270](stops at bad char 'a') @[3271-3274](fixed buffer of 70 chars? - jackpot!) @[3276](load buf w/chars before 'a' & keep track of prev char) @[3277-3287](remove underscores) @[3286](stop when n is at end of buf)
