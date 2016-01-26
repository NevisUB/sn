#!/usr/local/bin/python

import sys,os,commands,__main__

from subprocess import call
import time

### for plots: actual width is -1 wrt the one in the .txt

# continue from here

#for deltat in xrange(3, 64):
for deltat in xrange(13, 26):

    for tstart in xrange(192, 256 - deltat):
#  for tstart in xrange(0, 192, 10):
        if deltat <= 13 and tstart <= 218:
            continue

        cmd = "./mbtest_trial_case1_prod %i %i > tickScan.out 2>tickScan.err"%(tstart, tstart+deltat)
        
        print "** Processing ", cmd
        call(cmd,shell=True)
        time.sleep(2)
