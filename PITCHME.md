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
@title[Bug Script]

----?code=ruby_bug.rb&lang=ruby&title=Bug Script

---
@title[strtod()]

```C
double
     strtod(const char *restrict nptr, char **restrict endptr);
```
```
If endptr is not NULL, a pointer to the character after the last character used in the conversion is stored in the location referenced by endptr.

If no conversion is performed, zero is returned and the value of nptr is stored in the location referenced by endptr.
```

----
@title[Forensics]

[object.c](https://bugs.ruby-lang.org/projects/ruby-trunk/repository/revisions/63130/entry/object.c#L3232)
