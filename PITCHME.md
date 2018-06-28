---
@title[Introduction]

# Ruby Float() Bug

## Successfully converting an invalid string into a float

### Successfully converting an invalid string into a float

#### Successfully converting an invalid string into a float

Note:

- How bug was discovered

---

```
#!/usr/bin/env ruby

number = ""
bad_char = 'a'

def string_to_float(arg)
  Float(arg) rescue nil
end

1.upto(72).each do |j|
  number     = number + (j % 10).to_s
  bad_number = number + bad_char

  if flt = string_to_float(bad_number)
    puts "#{bad_number} len=#{number.size} val=#{flt}"
  else
    puts "#{bad_number} len=#{number.size} val=error"
  end
end
```

---?code=ruby_bug.rb&lang=ruby&title=Ruby Bug Discovery Script
