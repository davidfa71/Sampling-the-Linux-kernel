#!/bin/bash
# Usage: kconfigsampler.sh <system name> <number of samples>
if [[ $# -eq 0 ]]; then
  echo "Usage: kconfigsamplerme.sh <system name> <number of samples>"
  exit
fi
echo "Sampling "$1" with kconfigsampler for "$2" samples"
KconfigSampler $2 ../models/$1-manybdds.dddmp >../samples/$1-kconfigsampler-sample.txt 2>../samples/$1-kconfigsampler.log
