#!/bin/bash

# Configuration
CC=arm-linux-gnueabihf-gcc
TARGET=root@192.168.2.50
REMOTE_PATH=/root
OUTPUT=radar_brain
SRCS=radar_brain.c

# Flags from your command
LDFLAGS="-L. -lc_pluto -Wl,--dynamic-linker=/lib/ld-linux-armhf.so.3 -Wl,--allow-shlib-undefined -Wl,-rpath,/root,-lm"
LIBS="./libiio.so"

echo "--- Compiling $SRCS ---"
$CC $SRCS $LIBS -o $OUTPUT $LDFLAGS

if [ $? -eq 0 ]; then
    echo "--- Build Success! Uploading to Pluto ---"
    # -O uses the legacy SCP protocol which Pluto usually prefers
    scp -O $OUTPUT libiio.so $TARGET:$REMOTE_PATH/
    # ssh $TARGET ".$REMOTE_PATH/$OUTPUT" run the code remotely if needed
    echo "--- Upload Complete---"
else
    echo "--- Build FAILED ---"
    exit 1
fi