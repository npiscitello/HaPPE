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
  -i ../motion_detect_test/composite_bg/composite_bg.png \
  -itsoffset -0.1 -i ../motion_detect_test/walk2_10.mp4 \
  -i ../motion_detect_test/results/m_fgmask.mp4 \
  -i ../motion_detect_test/results/m_blurframe.mp4 \
  -i ../motion_detect_test/results/p_thresh.mp4 \
  -i ../motion_detect_test/results/p_blurframe.mp4 \
  -i ../motion_detect_test/results/result.mp4 \
  -filter_complex 'overlay=x=168:y=320,overlay=x=606:y=60,overlay=x=1044:y=60,overlay=x=606:y=580,overlay=x=1044:y=580,overlay=x=1482:y=320' \
  -pix_fmt yuv420p -an \
  ../motion_detect_test/composite_realtime.mp4

# slow down video - didn't really have the effect I thought it would
#ffmpeg \
#  -i ../motion_detect_test/composite_realtime.mp4 \
#  -filter "setpts=2.0*PTS" -r 30 \
#  ../motion_detect_test/composite_halfspeed.mp4
