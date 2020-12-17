CC = gcc
CFLAGS = -lm
TARGET = findLocC
SRCS = $(wildcard *.c)
OBJECTS = $(SRCS:.c=.o)
 
.SUFFIXES : .o .c
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $< -lm
 
all : $(TARGET)
 
$(TARGET) : $(OBJECTS)
	$(CC)  $(CFLAGS) -o $@ $^ -lm
 
clean :
	rm -rf *.o $(TARGET)
