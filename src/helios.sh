#! /bin/bash 
# @$-q lh10204 
# @$-g lh10204 
# @$-oi 
# @$-eo 
# @$-lP 2 
# @$-lp 16 
# @$-lm 28gb 
# @$-llm unlimited 
# @$-Pvn abs_unpack -Pvs unpack -Pvc unpack 
#
set -x


# for openmpi
. /thin/local/etc/setprofile/openmpi-1.3.3+gcc-4.1.sh
. /thin/local/etc/setprofile/intel-11.1.sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/b/b30683/blitz-gcc/lib/

#. /thin/local/etc/setprofile/openmpi-1.3.3+intel-11.0.sh
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/b/b30683/blitz-intel/lib/


cd $QSUB_WORKDIR 

export OMP_NUM_THREADS=4

mpiexec -n 4  -machinefile ${QSUB_NODEINF} ./helios -c /home/b/b30683/helios_ky/src/ETGShear_2D_Island.helios -f -d 4




