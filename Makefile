local: CC				= 	g++
local: CFLAGS		= 	-I/usr/local/include \
										-DVISIBLE_ALARM
local: LDFLAGS	= 	-L/usr/local/lib64 \
										-lopencv_core \
										-lopencv_imgcodecs \
										-lopencv_imgproc \
										-lopencv_features2d \
										-lopencv_videoio \

cross: CC 			=		arm-linux-gnueabihf-g++
cross: CFLAGS		= 	-I/opt/cross/rpi/rootfs/usr/include \
			    					-I/opt/cross/rpi/rootfs/usr/include/arm-linux-gnueabihf/ \
										-I/opt/cross/rpi/install/include/ \
										-DCROSS
cross: LDFLAGS 	=		-L/opt/cross/rpi/rootfs/usr/lib \
										-L/opt/cross/rpi/install/lib \
										-lopencv_core \
										-lopencv_imgcodecs \
										-lopencv_imgproc \
										-lopencv_videoio \
										-lopencv_features2d \
										-lopencv_flann \
										-lopencv_highgui


SRC_DIR					:= src
CSRCS						:= $(wildcard $(SRC_DIR)/*.cpp)

MAIN 						:= HaPPE

all: local

# this will call the same compilation target with the appropriate settings for a local build or a
# cross compile
.PHONY: local
local: build_tgt
.PHONY: cross
cross: build_tgt

# this is the top-level target - it should only be called as a prereq as the local or cross targets
# to make sure all the appropriate variables are set
build_tgt: $(MAIN)

# this is fragile - see the makefile in github.com/npiscitello/MegaWeather if you need to make it
# more complex
$(MAIN): $(SRC_DIR)/main.cpp
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

.PHONY: clean
clean:
	rm $(MAIN)
