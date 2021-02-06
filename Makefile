PROGRAM = game_demo_audio
OBJS = game_demo_audio.o azplf_audio.o wav_util.o
CC = arm-linux-gnueabihf-gcc
CFLAGS = -g -I./lib/include
LDFLAGS = -lpthread -L./lib -lazplf_hal -lazplf_util -lpng

all : $(PROGRAM)

$(PROGRAM) : $(OBJS) ./lib/libazplf_hal.so ./lib/libazplf_util.so
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

./lib/libazplf_hal.so: FORCE
	cd ${PWD}/lib/azplf_hal; make

./lib/libazplf_util.so: FORCE
	cd ${PWD}/lib/azplf_util; make

FORCE :

clean :
	rm -rfv *.o
	rm -rfv $(PROGRAM)
	rm -rfv ./lib/lib*.so

.PHONY : clean

# header file dependency

azplf_audio.o: azplf_audio.h
wav_util.o: wav_util.h
