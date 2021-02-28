# azplf: game_demo_audio
The azplf is a platform running on [ZYBO](https://reference.digilentinc.com/reference/programmable-logic/zybo/start) for helping fpga beginners to bring up a video/audio platform easily and users can connect their own hardware/software IPs to it. It includes game/video/audio HAL layer libraries. This is a simple game demo to learn how azplf works.
### Pre-requisites
#### Hardware  
1. ZYBO
2. LQ070 lcd display
3. Pmod to LQ070 extention board
4. Headphone

### How to build
1. Build an application on ZYBO  
```
$ sudo apt install gcc-4.7-arm-linux-gnueabihf
$ git clone https://github.com/atsupi/game_demo_audio.git
$ cd game_demo_audio
$ make clean
$ make
```
2. Copy boot files into SD card  
```
$ cp boot/* /mnt/ZYBO_BOOT
```

### How to run  
```
# cd game_demo_audio
# source setenv.sh
# ./game_demo_audio
```
### License
This work is licensed under a Creative Commons Zero v1.0 Universal License.

### Link
About ZYBO Embedded Linux, refer to the Digilent's blog [here](https://blog.digilentinc.com/zybo-embedded-linux-hands-on-tutorial/).
