#!/bin/bash

exe=$1
indir=$2
outdir=$3

if [[ ! -d $outdir ]]; then
  mkdir -p $outdir
fi

for file in $(ls $indir/*.in)
do
  filebase=$(basename ${file%.*})
  outfile=$outdir/${filebase}.out
  errfile=$outdir/${filebase}.err
  echo "$exe < $file > $outfile 2> $errfile"
  $exe < $file > $outfile 2> $errfile
  head $errfile
  tail $errfile
done