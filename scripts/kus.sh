#!/bin/bash 
# Usage: kus.sh <system name> <number of samples>
if [[ $# -eq 0 ]]; then
  echo "Usage: kusme.sh <system name> <number of samples>"
  exit
fi
echo "Sampling "$1" with kusme for "$2" samples"
python3 ../others/KUS-master/KUS.py --dDNNF $1.d4 --samples $2 --outputfile ../samples/$1-kus-sample.txt >../samples/$1-kus.log 2>&1
