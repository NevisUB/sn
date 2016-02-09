#!/usr/local/bin/python

import sys,os,commands,__main__

from subprocess import call
import time

### for plots: actual width is -1 wrt the one in the .txt

# continue from here

#for deltat in xrange(3, 64):
#for deltat in xrange(13, 26):

#for tstart in xrange(192, 256 - deltat):
#  for tstart in xrange(0, 192, 10):
#for tstart in xrange(240, 256 - deltat):
for tstart in xrange(245, 256):
#for tstart in xrange(243, 244):
 #       if deltat <= 13 and tstart <= 218:
  #          continue
	for tend in xrange(0, 256):

		# tend cannot be smaller than tstart
#		if tend < tstart:
#			continue
		# minimum width to test
#		if (tend - tstart) < 3:
#			continue
#		if (tstart == 243 and (tend - tstart) <= 7):
#			continue

#		if( (tend - tstart) == 6 or (tend + 256 - tstart) == 6 ):
		if(  (tend + 256 - tstart) == 6 ):

#		cmd = "./mbtest_trial_case1_prod %i %i > tickScan.out 2>tickScan.err"%(tstart, tstart+deltat)
			cmd = "./mbtest_trial_case1_prod %i %i > tickScan.out 2>tickScan.err"%(tstart, tend)
        
			print "** Processing ", cmd
			call(cmd,shell=True)
			time.sleep(2)
