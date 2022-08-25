#!/bin/bash
# Usage: unigen.sh <system name> <number of samples>
if [[ $# -eq 0 ]]; then
  echo "Usage: unigenme.sh <system name> <number of samples>"
  exit
fi
echo "Sampling "$1" with unigen for "$2" samples"
unigen --samples $2 --sampleout ../samples/$1-unigen-sample.txt ../models/$1.cnf >../samples/$1-unigen.log 2>&1
