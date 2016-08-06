# Script to run at Nevis

# Configuration parameters
Location=Nevis # Choose between Nevis, teststand, MicroBooNE
XMIT_FPGACode=/home/ub/xmit_fpga_link_header # Path to XMIT FPGA code
XMIT_Module=2 # Location of XMIT module in crate
FEM_FPGACode=/home/ub/module1x_140820_deb_3_21_2016.rbf # Path to FEM FPGA code with dynamic baseline
# FEM_FPGACode=/home/ub/module1x_140820_deb_fixbase_6_9_2016.rbf # Path to FEM FPGA code with fixed baseline
FEM_Module=3 # Location of FEM module in crate
OutputTriggerFile=test_trig.dat # Name of output file for trigger stream (date will be prepended)
OutputSupernovaFile=test_snova.dat # Name of output file for supernova stream (date will be prepended)
AmplitudeThreshold=10 # Amplitude threshold in ADC
FixBaseline=0 # Use fixed baseline: 0 = no, 1 = yes (FEM FPGA code must allow it)
BaselinesFile=/root/aaa/channelBaselinesNevis.txt # Path to file with channel baselines
BaselineMeanTolerance=4095 # Maximum difference between means to establish baseline
BaselineVarianceTolerance=4095 # Maximum difference between variances to establish baseline
Presamples=2 # Number of presamples
Postsamples=3 # Number of postamples
Bipolar=0 # Bipolar mode (0: signal is above threshold, 2: signal is either above or below threshold)

# Generate a copy of this script (identified by its timestamp) as log
logname=`date +%Y%m%d%H%M%S`"_"`basename "$0" .sh`".log"
cp $0 $logname

# Run Nevis DAQ recipe
# If a parameter is added, add it last and edit mbtest_2stream_real.c accordingly
./generic_2stream_real $Location $XMIT_FPGACode $XMIT_Module $FEM_FPGACode $FEM_Module $OutputTriggerFile $OutputSupernovaFile $AmplitudeThreshold $FixBaseline $BaselinesFile $BaselineMeanTolerance $BaselineVarianceTolerance $Presamples $Postsamples $Bipolar