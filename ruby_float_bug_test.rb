#!/usr/bin/env ruby

require 'test/unit'
require 'test/unit/assertions'
include Test::Unit::Assertions

class TestFloat < Test::Unit::TestCase

  # https://bugs.ruby-lang.org/projects/ruby-trunk/repository/entry/object.c#L3271
  # BUF_SIZE = 69 on most machines
  # -1 is for newline
  # Bonus points if you can explain the constants 4 and 10?
  BUF_SIZE = Float::DIG * 4 + 10 - 1

  # case 1: invalid char 'a' is within buffer size
  # Result: strtod correctly throws error
  def test_strtod_ok
    assert_raise(ArgumentError){Float('1' * (BUF_SIZE-1) + 'a')}
  end

  # case 2: invalid char 'a' is outside buffer size
  # Result: strtod doesn't throw error because buffer doesn't contain invalid char.
  # Confusing why ruby's behavior is different between case 1 and 2 until you look at C code.
  def test_strtod_no_error
    assert_equal(1.1111111111111112e+68, Float('1' * BUF_SIZE + 'a is ignored'))
  end

  # case 3: entire string is scanned for underscores and verified prev char ISDIGIT.
  # Result: when '_' is found in string, prev char is checked and MUST be ISDIGIT
  # or error is thrown by rb_cstr_to_dbl_raise not strtod.
  def test_underscores_checked_whole_string
    assert_raise(ArgumentError){Float('1' * BUF_SIZE  + '234_56a_890')}
  end

  # case 4: the bug - ruby should scan entire string and detect invalid chars
  # just like it does for invalid underscores so this test should pass.
  # Result: no exception raised
  def test_check_whole_string
    assert_raise(ArgumentError){Float('1' * BUF_SIZE  + 'a')}
  end

end
