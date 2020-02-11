#!/bin/bash

run=$1
indir=$2
outdir=$3
checker="dummy"
if [[ $# -eq 4 ]]; then
  checker=$4
fi

if [[ ! -d $outdir ]]; then
  mkdir -p $outdir
fi

result=$3/result.txt
echo -n > $result

for i in $(seq 1 10)
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
done
cat $result
