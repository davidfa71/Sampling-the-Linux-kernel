#!/bin/bash
# Usage: all.sh: <timeout> <number of samples>
if [[ $# -eq 0 ]]; then
  echo "Usage: allme.sh <timeout> <number of samples>"
  exit
fi
for s in axtls buildroot busybox coreboot embtoolkit fiasco freetz linux toybox uClibc
   do
   echo "Sampling "$s
   timeout $1 ./unigen.sh         $s $2 &
   timeout $1 ./spur.sh           $s $2 &
   timeout $1 ./quicksampler.sh   $s $2 &
   timeout $1 ./kus.sh            $s $2 &
   timeout $1 ./kconfigsampler.sh $s $2 &
   timeout $1 ./smarch.sh         $s $2 &
   done
wait
