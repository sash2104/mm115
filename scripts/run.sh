#!/bin/bash

run=$1
rootdir=$2
indir=$rootdir/data
outdir=$3
checker="dummy"
if [[ $# -eq 4 ]]; then
  checker=$4
fi
logger=$rootdir/scripts/logger.py
visualizer=$rootdir/scripts/visualizer.py


if [[ ! -d $outdir ]]; then
  mkdir -p $outdir
fi

result=$3/result.txt
echo -n > $result

for i in $(seq 1 2)
do
  file=$indir/$i.in
  filebase=$(basename ${file%.*}) # 現状はiと同じになる
  outfile=$outdir/${filebase}.out
  errfile=$outdir/${filebase}.err
  echo "$run < $file > $outfile 2> $errfile"
  time $run < $file > $outfile 2> >(tee $errfile >&2)
  # tail $errfile
  if [[ $# -eq 4 ]]; then
    ansfile=$indir/$i.ans
    $checker $file $outfile $ansfile | sed "s/{/{\"seed\":\"$i\",/" >> $result
  fi

  # visualize result
  if [[ -f $visualizer ]]; then
    python $visualizer -i $file -o $outfile -a $ansfile -p $outdir/$i.png
  fi
done

cat $result
