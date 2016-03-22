import os
os.system("hexdump -e '8/4 \"%08X \"\"\n\"' test123_pt_trig.dat > trigdata.dat")
os.system("hexdump -e '8/4 \"%08X \"\"\n\"' test123_pt_snova.dat > sndata.dat")


sndata = trdata =None

with open("sndata.dat") as f:
    sndata = f.read()

with open("trigdata.dat") as f:
    trdata = f.read()

snwords = [i.split(" ") for i in sndata.split("\n")]
snwords = [item for sublist in snwords for item in sublist]

trwords= [i.split(" ") for i in trdata.split("\n")]
trwords = [item for sublist in trwords for item in sublist]

ahosn = 0
ahotr = 0
for ix,word in enumerate(snwords):
    word = word.rstrip()
    if word == 'FFFFFFFF':
        print ix,snwords[ix],snwords[ix+1],snwords[ix+2],snwords[ix+3],snwords[ix+4],snwords[ix+5],snwords[ix+6]

print 'aho'
print 'aho'
print 'aho'
for ix,word in enumerate(trwords):
    word = word.rstrip()
    if word == 'FFFFFFFF':
        print ix,trwords[ix],trwords[ix+1],trwords[ix+2],trwords[ix+3],trwords[ix+4],trwords[ix+5],trwords[ix+6]
        
print ahosn,ahotr


