# Python script to periodically monitor the size growth rate of two files given as argv[2] and argv[3], writing it to a file given as argv[1] 

import os, time, sys

with open(sys.argv[1], 'w') as outfile:
    while(1) :
        size1_sn = 0.0
        size1_tr = 0.0
        size2_sn = 0.0
        size2_tr = 0.0
        snfile=sys.argv[2]
        trigfile=sys.argv[3]
        
        time.sleep(1)
        size1_sn = os.path.getsize(snfile)
        size1_tr = os.path.getsize(trigfile)
        time.sleep(2.0)
        size2_sn = os.path.getsize(snfile)
        size2_tr = os.path.getsize(trigfile)
        
        datarate_sn = (float(size2_sn) - float(size1_sn)) / 2000000.
        datarate_tr = (float(size2_tr) - float(size1_tr)) / 2000000.
        datarate_total = datarate_sn + datarate_tr
        
        outfile.write("\n\nSN data rate (MB/s) %f" % datarate_sn)
        outfile.write("\nTrigger data rate (MB/s) %f" % datarate_tr)
        outfile.write("\nTotal data rate (MB/s) %f\n" % datarate_total)
        outfile.flush()
