CC 					:= g++
CFLAGS			:= -I/home/npiscitello/custom_build/include
LDFLAGS			:= -L/home/npiscitello/custom_build/lib64 \
								-lopencv_core \
								-lopencv_imgcodecs \
								-lopencv_imgproc

%: %.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
