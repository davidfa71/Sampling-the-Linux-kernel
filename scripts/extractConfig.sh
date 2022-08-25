#!/bin/bash
if [ ! $# -eq 2 ]
then 
  echo $0 ": <num> <configurations file>"
  echo "Extracts configuration number num from the file (starts at 1)"
  exit 1
fi
LINES=$(wc -l <$2)
if (( $1 >  0 ))  && (( $1 <= $LINES ))
then
  sed ${1}'q;d' $2 | awk '{ for(i=1; i <= NF; i++) if ($i ~ "\=n")  printf("# %s is not set\n",substr($i, 0, length($i)-2)); else printf("%s\n", $i); }'
else
  echo Wrong line number $1
fi
