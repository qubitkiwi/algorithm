CC=gcc
CFLAGS=-g -Wall
OBJS=main.o btree.o
TARGET = btree

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f *.o
	rm -f $(TARGET)