#!/bin/bash

if [ $# -lt 5 ]
then
  echo "Usage: ./concat.sh <full ppe> <no gloves> <no glasses> <no ppe> <concat output>"
  exit 1
fi

ffmpeg \
  -i $1 \
  -i $2 \
  -i $3 \
  -i $4 \
  -filter_complex \
    "[0:v]drawtext=fontfile=Ubuntu-r.ttf:text='Full PPE':fontcolor='white':fontsize=70:x=20:y=20 [full];
    [1:v]drawtext=fontfile=Ubuntu-r.ttf:text='No Gloves':fontcolor='white':fontsize=70:x=20:y=20 [gloves];
    [2:v]drawtext=fontfile=Ubuntu-r.ttf:text='No Glasses':fontcolor='white':fontsize=70:x=20:y=20 [glasses];
    [3:v]drawtext=fontfile=Ubuntu-r.ttf:text='No PPE':fontcolor='white':fontsize=70:x=20:y=20 [none];
    [full][gloves][glasses][none]concat=n=4" \
  $5


