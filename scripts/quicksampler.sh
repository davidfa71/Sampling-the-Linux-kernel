#!/bin/bash
# Usage: quicksampler.sh <system name> <number of samples>
if [[ $# -eq 0 ]]; then
  echo "Usage: quicksamplerme.sh <system name> <number of samples>"
  exit
fi
echo "Sampling "$1" with quicksampler for "$2" samples"
../others/quicksampler/quicksampler/quicksampler -n $2 ../models/$1.cnf >../samples/$1-quicksampler.log 2>&1
mv $1.cnf.samples ../samples/$1-quicksampler-sample.txt
