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
# in1: source video, centered at (33,180), scaled up by 1.5x, must be advanced by one frame due to processing
# in2: motion thresh, centered at (606,60)
# in3: motion blobs, centered at (1044,60)
# in4: ppe thresh, centered at (606,580)
# in5: ppe blobs, centered at (1044,580)
# in6: result, centered at (1482,320), scaled up by 1.5x
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
  -filter_complex  '[1:v]scale=406:720 [src];
                    [6:v]scale=406:720 [res];
                    [0:v][src]overlay=x=32:y=180 [s1];
                    [s1][2:v]overlay=x=606:y=60 [s2],
                    [s2][3:v]overlay=x=1044:y=60 [s3],
                    [s3][4:v]overlay=x=606:y=580 [s4],
                    [s4][5:v]overlay=x=1044:y=580 [s5],
                    [s5][res]overlay=x=1482:y=180' \
  -pix_fmt yuv420p -an \
  $2

# this is the loop filter - add an output to the last overlay to pipe into here
#[full]loop=4:200'\

# remove working folder
rm -r $PROC_DIR
