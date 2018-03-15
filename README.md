# HaPPE: Heuristic Alerts for Personal Protective Equipment
Due to forgetful or dismissive behavior, personal protective equipment is often lacking or misused
in environments that require it. To counter this behavior, a system to monitor and alert users of
their neglected PPE when entering designated regions is being developed. This system will use vision
processing to detect potentially missing PPE and trigger an alarm system to remind the user to use
correct protection.

## Cross-Compiling OpenCV for the RPi
See this SO answer for general cross compilation setup: https://stackoverflow.com/a/19269715

### Build Prep
libx264: ./configure --prefix=../install/ --cross-prefix=arm-linux-gnueabihf- --host=arm-linux-gnueabihf
ffmpeg: ./configure --enable-cross-compile --cross-prefix=arm-linux-gnueabihf- --arch=armel --target-os=linux --prefix=../install --extra-cflags="-I/path/to/cross/compiled/libx264/include" --extra-ldflags="-L/path/to/cross/compiled/libx264/lib" --extra-libs=-ldl
opencv: cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=../../install/ -D CMAKE_TOOLCHAIN_FILE=../platforms/linux/arm-gnueabi.toolchain.cmake ..
* must add "-mcpu=cortex-a7 -mfloat-abi=hard -mfpu=vfpv4" to the cmake file - e.g.
  `set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=vfpv4")`
  `set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=vfpv4")`
  (see https://stackoverflow.com/a/33280037)
* the above instructions produce a vanilla opencv (with NO ffmpeg) - I still haven't gotten an
  openCV with ffmpeg build to compile

# ToDo

* Can we license under the GPL? How much ownership does Wentworth have over this?
