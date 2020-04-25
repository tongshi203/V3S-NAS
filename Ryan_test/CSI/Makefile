.SUFFIXES : .x .o .c .s

CC := arm-linux-gnueabihf-gcc

TARGET = v4l2_test
SRCS := v4l2_raw.c
LIBS= -lc -lgcc -lc -lpthread

all: 
	$(CC) -static $(SRCS) -o $(TARGET)

	cp v4l2_test ../../rootfs/sbin/
clean:
	rm -f *.o 
	rm -f *.x 
	rm -f *.flat
	rm -f *.map
	rm -f temp
	rm -f *.img
	rm -f $(TARGET)	
	rm -f *.gdb
	rm -f *.bak
