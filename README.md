CM-R1


arm-linux-gnueabihf-gcc radar_brain.c ./libiio.so -o radar_brain -L. -lc_pluto -Wl,--dynamic-linker=/lib/ld-linux-armhf.so.3 -Wl,--allow-shlib-undefined -Wl,-rpath,/root && scp -O radar_brain libiio.so root@192.168.2.1:/root/

Compile and transfer to the pluto. 
Then from pluto, chmod +x the new executable and run.
