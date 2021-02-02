LIBS = libazplf_util.so
OBJS = bitmap.o png_util.o
CC = arm-linux-gnueabihf-gcc
CFLAGS = -g -shared -fPIC -I../include 
LFLAGS = 

all : $(LIBS)
	cp ${LIBS} ../${LIBS}

$(LIBS) : $(OBJS)
	${CC} ${CFLAGS} $^ -o $@ ${LFLAGS}

clean :
	rm -rfv *.o
	rm -rfv $(LIBS)

.PHONY : clean

# header file dependency

bitmap.o: ../include/bitmap.h
png_util.o: ../include/bitmap.h ../include/png_util.h
