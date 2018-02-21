CC 					:= g++
CFLAGS			:= -I/usr/local/include
LDFLAGS			:= -L/usr/local/lib64 \
								-lopencv_core \
								-lopencv_imgcodecs \
								-lopencv_imgproc \
								-lopencv_video \
								-lopencv_videoio

%: %.cpp
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@
