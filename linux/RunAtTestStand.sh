# Script to run at LArTF test stand

# Configuration parameters
Location=teststand # Choose between Nevis, teststand, MicroBooNE
XMIT_FPGACode=/home/jcrespo/fpga/readcontrol_110601_v3_play_header_8_19_2013.rbf # Path to XMIT FPGA code
XMIT_Module=6 # Location of XMIT module in crate
FEM_FPGACode=/home/jcrespo/fpga/module1x_140820_deb_nf_pol_1_5_2017.rbf # Path to FEM FPGA code with channelwise polarity and 14-bit time
# FEM_FPGACode= /home/jcrespo/fpga/module1x_140820_deb_fixbase_6_9_2016.rbf # Path to FEM FPGA code with fixed baseline
FEM_Module=9 # Location of leftmost FEM module in crate to be read (this and all FEMs between this one and the XMIT will be used)
OutputTriggerFile=test_trig_3mod.dat # Name of output file for trigger stream (date will be prepended)
OutputSupernovaFile=test_snova_3mod.dat # Name of output file for supernova stream (date will be prepended)
AmplitudeThreshold=0 # NOT USED # Amplitude threshold in ADC
FixBaseline=0 # Use fixed baseline: 0 = no, 1 = yes (FEM FPGA code must allow it)
BaselinesFile=0 # Path to file with channel baselines
BaselineMeanTolerance=2 # Maximum difference between means to establish baseline
BaselineVarianceTolerance=3 # Maximum difference between variances to establish baseline
Presamples=7 # Number of presamples
Postsamples=7 # Number of postamples
Bipolar=0 # NOT USED # Bipolar mode (0: signal is above threshold, 2: signal is either above or below threshold)

# Generate a copy of this script (identified by its timestamp) as log
logname=`date +%Y%m%d%H%M%S`"_"`basename "$0" .sh`".log"
cp $0 $logname
# Add machine name to log to identify crate
machname="\n\nMachine="`uname -n`"\n"
echo -e $machname >> $logname

# Run Nevis DAQ recipe
# If a parameter is added, add it last and edit mbtest_2stream_real.c accordingly
./generic_2stream_real $Location $XMIT_FPGACode $XMIT_Module $FEM_FPGACode $FEM_Module $OutputTriggerFile $OutputSupernovaFile $AmplitudeThreshold $FixBaseline $BaselinesFile $BaselineMeanTolerance $BaselineVarianceTolerance $Presamples $Postsamples $Bipolar