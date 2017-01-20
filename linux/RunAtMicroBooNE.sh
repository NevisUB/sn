# Script to run at MicroBooNE

# Configuration parameters
Location=MicroBooNE # Choose between Nevis, teststand, MicroBooNE
XMIT_FPGACode=/home/kterao/local152_code/xmit_fpga_link_header # Path to XMIT FPGA code
#XMIT_Module=7 # Location of XMIT module in crate for SEB 01
XMIT_Module=3 # Location of XMIT module in crate for SEB 02-08
#XMIT_Module=4 # Location of XMIT module in crate for SEB 09
FEM_FPGACode=/home/davidc1/firmware/module1x_140820_deb_3_21_2016.rbf # Path to FEM FPGA code
#FEM_FPGACode=/home/jcrespo/fpga/module1x_140820_deb_fixbase_6_9_2016.rbf # Path to FEM FPGA code with fixed baseline
FEM_Module=18 # Location of leftmost FEM module in crate to be read (this and all FEMs between this one and the XMIT will be used)
OutputTriggerFile=test_trig_fullCrate.dat # Name of output file for trigger stream (date will be prepended)
OutputSupernovaFile=test_snova_fullCrate.dat # Name of output file for supernova stream (date will be prepended)
AmplitudeThreshold=5 # Amplitude threshold in ADC
FixBaseline=0 # Use fixed baseline: 0 = no, 1 = yes (FEM FPGA code must allow it)
#FixBaseline=1 # Use fixed baseline: 0 = no, 1 = yes (FEM FPGA code must allow it)
BaselinesFile=0 # Path to file with channel baselines
#BaselinesFile=/home/jcrespo/sn_beamdown/baselines/channelBaselines_Crate08_FEM04_good.txt # Path to file with channel baselines
#BaselinesFile=/home/jcrespo/sn_beamdown/baselines/channelBaselines_Crate02_FEM04_good.txt # Path to file with channel baselines
BaselineMeanTolerance=1 # Maximum difference between means to establish baseline
BaselineVarianceTolerance=1 # Maximum difference between variances to establish baseline
Presamples=63 # Number of presamples
Postsamples=31 # Number of postamples
Bipolar=0 # Bipolar mode (0: signal is above threshold, 2: signal is either above or below threshold)

# Generate a copy of this script (identified by its timestamp) as log
logname=`date +%Y%m%d%H%M%S`"_"`basename "$0" .sh`".log"
cp $0 $logname
# Add machine name to log to identify crate
machname="\n\nMachine="`uname -n`"\n"
echo -e $machname >> $logname

# Run Nevis DAQ recipe
# If a parameter is added, add it last and edit mbtest_2stream_real.c accordingly
./generic_2stream_real $Location $XMIT_FPGACode $XMIT_Module $FEM_FPGACode $FEM_Module $OutputTriggerFile $OutputSupernovaFile $AmplitudeThreshold $FixBaseline $BaselinesFile $BaselineMeanTolerance $BaselineVarianceTolerance $Presamples $Postsamples $Bipolar