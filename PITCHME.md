---
@title[Introduction]

# Ruby Float() Bug

#### Converting an invalid string into a float

#### Sam Napolitano

Note:

- How bug was discovered

---
@title[Float examples]
Float examples

```ruby
irb(main):0:0> Float("3.14")
=> 3.14
irb(main):0:0> Float("3.14a")
ArgumentError: invalid value for Float(): "3.14a"
irb(main):0:0> Float("0xFF")
=> 255.0
```

Note:

- Noticed that really long string was valid but mistyped character and it was still valid

---
@title[ruby_bug.rb output on ruby 2.2.7]
Script output in 2.2.7

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

---?code=ruby_bug.rb&lang=ruby&title=ruby_bug.rb
ruby_bug.rb

@[16-17](initialize)
@[19-21](convert string to float)
@[23-32](loop growing string one digit at a time)

---
@title[strtod specification]
strtod() specification

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
strtod() examples

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
@title[strtod()]

```C
double
     strtod(const char *restrict nptr, char **restrict endptr);
```

@css[left-justified]

>If endptr is not NULL, a pointer to the character after the last character used in the conversion is stored in the location referenced by endptr.

>If no conversion is performed, zero is returned and the value of nptr is stored in the location referenced by endptr.

---?code=presentation/object-v63130.c&lang=c&title=object-v63130.c
object.c v63130

@[3231-3232](p=string, badcheck=true, raise=true, error=NULL)
@[3252](first attempt to convert string)
@[3270]('end' is pointing to bad char 'a')
@[3271-3274](fixed buffer? Hmm... what size?)
@[3276](load buf with chars stopping at 'a'; keep track of prev char)
@[3277-3284](remove underscore; TIL 8_00_0 is valid)
@[3286](stop when n is at end of buf)
@[3288-3289](terminate string and assign p)
@[3295](convert string using normalized buffer)

Note:

- DBL_DIG is usually 15
- buf is 70
- p points to string passed in
- n points to buf
- e points to end of buf w/one space for NULL
- prev points to prev char of p
- buf is set with prev char IFF n < e

---?code=presentation/object.c&lang=c&title=object.c on trunk
object.c on trunk

@[3269-3275](additional new locals)
@[3277-3281](handle +/- and leading zeros)
@[3291](assign prev to current char)
@[3292-3301](handle exponentiation validation)
@[3302-3308](trailing whitespace)
@[3309](validate one dot & validate digits)
@[3313-3316, 3322](same as before)

---
@title[Bug Script Output 2.6preview2]
Script output in 2.6preview2

```ruby
1a len=1 val=error
12a len=2 val=error
123a len=3 val=error
...
1234567890123456789012345678901234567890123456789012345678901234567a len=67 val=error
12345678901234567890123456789012345678901234567890123456789012345678a len=68 val=error
123456789012345678901234567890123456789012345678901234567890123456789a len=69 val=error
1234567890123456789012345678901234567890123456789012345678901234567890a len=70 val=error
```

Note:

- have minitest but using this for clarity

---
@title[Questions]

### Questions?

@size[20px](https://github.com/samiam/ruby_bug_14729)
