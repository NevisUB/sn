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

Generic_2stream_real.c is a version of mbtest 2 stream code for real data with local dependences removed
Configuration is done using a shell script. Three examples are provided:
  * RunAtNevis.sh -> To run at Nevis test stand.
  * RunAtTestStand.sh -> To run at LArTF test stand
  * RunAtMicroBooNE.sh -> To run at MicroBooNE
  
The makefiles are separated since WinDriver installation is different on each location:
  * At Nevis use makefile_genericNevis_2stream_real
  * At LArTF test stand or MicroBooNE use makefile_genericLArTF_2stream_real
