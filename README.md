# sn
Nevis trigger &amp; supernova stream readout 

At Nevis, PCIe cards are automatically identified by Device ID. At LArTF, both at the test stand and in MicroBooNE, all PCIe cards share the Device ID and must be identified manually.

## For LArTF test stand:
  * Controller card has "Location: Bus 0x4"
  * SN stream card has "Location: Bus 0x9"
  * NU stream card has "Location: Bus 0x5"
  
So when running there, the devices must be selected in order **1, 3, 2**

## For MicroBooNE:
  * Controller card has "Location: Bus 0x9"
  * SN stream card has "Location: Bus 0x4"
  * NU stream card has "Location: Bus 0x5"
  
So when running there, the devices must be selected in order **3, 1, 2**

In all locations, **fake data must be run before running with real data**

Generic\_2stream\_real.c is a version of mbtest 2 stream code for real data with local dependences removed.
Configuration is done using a shell script. Three examples are provided:
  * RunAtNevis.sh -> To run at Nevis test stand.
  * RunAtTestStand.sh -> To run at LArTF test stand
  * RunAtMicroBooNE.sh -> To run at MicroBooNE
  
The makefiles are separated since WinDriver installation is different on each location:
  * At Nevis use makefile\_genericNevis\_2stream\_real
  * At LArTF test stand or MicroBooNE use makefile\_genericLArTF\_2stream\_real

How to set up the LArTF test stand and MicroBooNE machines:
```
source /uboonenew/setup_online.sh # set up your environment

setup windriver v11_00_01 -q debug:e6 # set up windriver
```
Consider adding them to your .bashrc file if you use them often.

###### Frequent issues:

* Windriver license issue. Error looks like:
```
PCIE diagnostic utility.
Application accesses hardware using WinDriver.
pcie_diag: Failed to initialize the PCIE library: Failed to initialize the WDC library. Error 0x20000001 - Invalid handle
```
Solution: become root (you need to be added to the list, open a ticket using the Fermilab Service Desk if needed), and renew the licence:
```
ksu
source /etc/rc.local
exit
```

###### Useful comands:
Dump the binary data into 8 columns of 4-byte (32 bits) zero-padded 8-character-wide hexadecimal words.
```
hexdump -v -e '8/4 "%08X " "\n"'
```
