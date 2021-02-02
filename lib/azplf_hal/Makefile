LIBS = libazplf_hal.so
OBJS = azplf_hal_main.o vdma.o gfxaccel.o lq070out.o font.o sprite.o game.o
CC = arm-linux-gnueabihf-gcc
CFLAGS = -g  -shared -fPIC -I../include

all : $(LIBS)
	cp ${LIBS} ../${LIBS}

$(LIBS) : $(OBJS)
	${CC} ${CFLAGS} $^ -o $@

clean :
	rm -rfv *.o
	rm -rfv $(LIBS)

.PHONY : clean

# header file dependency

azplf_hal_main.o: ../include/azplf_hal.h

# video processing
vdma.o: ../include/vdma.h
lq070out.o: ../include/lq070out.h

# graphics processing
gfxaccel.o: ../include/gfxaccel.h
font.o: ../include/font.h
sprite.o: ../include/sprite.h

# game core processing
game.o: ../include/game.h
