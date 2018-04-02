#!/bin/bash

# Usage: ./process.sh <raw video input> <processed video output>
# must be run from the root directory of the git repo

if [ $# -lt 2 ]
then
  echo "Usage: ./process.sh <raw video input> <processed video output>"
  echo "Note: Must be run from the root of the git repo after building the main application ('make')"
  echo "Note: Optimized for vertically oriented 16:9 sources, though other sizes will work"
  exit 1
fi

# make working folder
export PROC_DIR=process
mkdir -p $PROC_DIR

# convert source to the correct size and framerate
ffmpeg -i $1 -r 10 -s 270x480 $PROC_DIR/raw.mp4

# process raw videoes
echo "Processing video, this could take a while..."
export FRAMES_DIR=frames
./HaPPE $PROC_DIR/frames $PROC_DIR/raw.mp4

# generate composite video
# origin at top left corner
# in0: composite background, makes up bounds
# in1: source video, centered at (168,320), must be advanced by one frame due to processing
# in2: motion thresh, centered at (606,60)
# in3: motion blobs, centered at (1044,60)
# in4: ppe thresh, centered at (606,580)
# in5: ppe blobs, centered at (1044,580)
# in6: result, centered at (1482,320)
# overlay everything
# set the pixel format to play nice with video players, strip the audio
ffmpeg \
  -i composite_bg/composite_bg.png \
  -itsoffset -0.1 -i $PROC_DIR/raw.mp4 \
  -r 10 -i $PROC_DIR/$FRAMES_DIR/m_fgmask/%d.png \
  -r 10 -i $PROC_DIR/$FRAMES_DIR/m_blurframe/%d.png \
  -r 10 -i $PROC_DIR/$FRAMES_DIR/p_thresh/%d.png \
  -r 10 -i $PROC_DIR/$FRAMES_DIR/p_blurframe/%d.png \
  -r 10 -i $PROC_DIR/$FRAMES_DIR/result/%d.png \
  -filter_complex 'overlay=x=168:y=320,overlay=x=606:y=60,overlay=x=1044:y=60,overlay=x=606:y=580,overlay=x=1044:y=580,overlay=x=1482:y=320' \
  -pix_fmt yuv420p -an \
  $2

# remove working folder
rm -r $PROC_DIR
