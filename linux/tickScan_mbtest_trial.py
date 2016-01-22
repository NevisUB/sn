#!/usr/local/bin/python

import sys,os,commands,__main__

from subprocess import call
import time

### for plots: actual width is -1 wrt the one in the .txt

# continue from here
#for width in xrange(25,30):
#   for tstart in xrange(192,256-width): #max tstart is 254
#      if width == 25 and tstart <= 223:
#         continue

for width in xrange(3,30):
   tstart = 255 - width
   
   cmd = "./mbtest_trial_case1_prod %i %i > tickScan.out 2>tickScan.err"%(tstart, tstart+width)
   
   print "** Processing ", cmd
   call(cmd,shell=True)
   time.sleep(1)
