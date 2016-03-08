import os, time

print "\t==>du watch<=="
while(1) :
    size1 = 0.0
    size2 = 0.0

    time.sleep(1)
    size1 = os.path.getsize("test123_pt_snova.dat")
    time.sleep(2)
    size2 = os.path.getsize("test123_pt_snova.dat")
    print str ( ( float(size2) - float(size1) ) / 2.0 / 1000000.0 ) + " MB/s"

