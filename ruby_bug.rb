#!/usr/bin/env ruby

i = 1
str = ""
#str = "1"
#str = "1.2"
#bad_char = 'a'
bad_char = '.7a'

def convert?(arg)
  true if Float(arg) rescue false
end

1.upto(72) do

  str += i.to_s
  str_b = str + bad_char

  if convert?(str_b)
    flt = Float(str_b)
    puts "#{str_b} len=#{str.size} val=#{flt}"
  else
    puts "#{str_b} len=#{str.size} val=0"
  end

  i = i.succ % 10
end
