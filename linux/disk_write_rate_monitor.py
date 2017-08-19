import os, time

while(1) :
    size1_sn = 0.0
    size1_tr = 0.0
    size2_sn = 0.0
    size2_tr = 0.0
    snfile="fakedata_snova.dat"
    trigfile="fakedata_trig.dat"

    time.sleep(1)
    size1_sn = os.path.getsize(snfile)
    size1_tr = os.path.getsize(trigfile)
    time.sleep(2)
    size2_sn = os.path.getsize(snfile)
    size2_tr = os.path.getsize(trigfile)

    datarate_sn = (float(size2_sn) - float(size1_sn)) / 1000000.
    datarate_tr = (float(size2_tr) - float(size1_tr)) / 1000000.
    datarate_total = datarate_sn + datarate_tr

    print "\n\nSN data rate (MB/s) ", datarate_sn
    print "Trigger data rate (MB/s)", datarate_tr
    print "Total data rate (MB/s)", datarate_total
