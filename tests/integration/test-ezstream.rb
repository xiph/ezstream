#!/usr/bin/env ruby
#
# encoding: UTF-8

MYNAME = File.basename($PROGRAM_NAME, '.rb')
EZSTREAM = '../../src/ezstream'
ICECAST = 'icecast2'

require 'open3'
require 'timeout'

def test_help
  STDERR.puts 'test_help'
  exit_status = 0
  Open3.popen3(EZSTREAM, '-h') do |i, o, e, wait_thr|
    exit_status = wait_thr.value.exitstatus
  end
  fail "exit_status: 0 != #{exit_status}" if exit_status != 0
  1
rescue => e
  STDERR.puts 'test_help: ' + e.message
  0
end

def test_stream
  STDERR.puts 'test_stream: http://127.0.0.1:34533/test1.ogg'
  exit_status = 0
  Open3.popen3(ICECAST, '-c', 'icecast.xml') do |ii, io, ie, icecast|
    sleep(2) # icecast needs time to boot
    Open3.popen3(EZSTREAM, '-v', '-v', '-v', '-c', 'ezcfg-test1.xml') do |ei, eo, ee, ezstream|
      exit_status += ezstream.value.exitstatus
      puts ee.read.chomp
    end
    sleep(1)
    Process.kill(15, icecast.pid)
  end
  fail "exit_status: 0 != #{exit_status}" if exit_status != 0
  1
rescue => e
  STDERR.puts 'test_stream: ' + e.message
  0
end

num_tests = 0
num_passed = 0

num_tests += 1
num_passed += test_help

num_tests += 1
num_passed += test_stream

success_rate = num_passed.to_f.fdiv(num_tests.to_f) * 100

STDERR.puts "#{num_passed}/#{num_tests} passed (#{(success_rate.round)}%)"

exit num_tests == num_passed ? 0 : 1
