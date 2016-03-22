import os, time

print "\t==>du watch<=="
while(1) :
    size1_sn = 0.0
    size1_tr = 0.0
    size2_sn = 0.0
    size2_tr = 0.0

    time.sleep(1)
    size1_sn = os.path.getsize("test123_pt_snova.dat")
    size1_tr = os.path.getsize("test123_pt_trig.dat")
    time.sleep(2)
    size2_sn = os.path.getsize("test123_pt_snova.dat")
    size2_tr = os.path.getsize("test123_pt_trig.dat")

    print str ( ( float(size2_sn) - float(size1_sn)  + float(size2_tr) - float(size1_tr)) / 2.0 / 1000000.0 ) + " MB/s"

