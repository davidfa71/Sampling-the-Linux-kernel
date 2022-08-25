#!/bin/bash
# Usage: smarch.sh <system name> <number of samples>
if [[ $# -eq 0 ]]; then
  echo "Usage: smarchme.sh <system name> <number of samples>"
  exit
fi
echo "Sampling "$1" with smarchme for "$2" samples"
python3 ../others/Smarch-master/smarch_base.py -o . ../models/$1.cnf $2 >../samples/$1-smarch.log 2>&1
mv $1_$2.samples ../samples/$1-smarch-sample.txt
