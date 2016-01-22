#!/usr/local/bin/python

import sys,os,commands,__main__

from subprocess import call
import time

### for plots: actual width is -1 wrt the one in the .txt

# continue from here

for width in xrange(28,64):

  for tstart in xrange(192,256-width): #max tstart is 254

# for width in xrange(39,64):
#    tstart = 255 - width
   
    if width == 28 and tstart <= 207:
      continue

    cmd = "./mbtest_trial_case1_prod %i %i > tickScan.out 2>tickScan.err"%(tstart, tstart+width)
   
    print "** Processing ", cmd
    call(cmd,shell=True)
    time.sleep(1)
