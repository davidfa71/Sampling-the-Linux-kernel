#!/bin/bash
# Usage: spur.sh <system name> <number of samples>
if [[ $# -eq 0 ]]; then
  echo "Usage: spurme.sh <system name> <number of samples>"
  exit
fi
echo "Sampling "$1" with spurme for "$2" samples"
../others/spur-master/build/Release/spur -cnf ../models/$1.cnf  -s $2 -out ../samples/$1-spur-sample.txt >../samples/$1-spur.log 2>&1
