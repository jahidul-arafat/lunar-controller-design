CC = gcc
CFLAGS = -Wall -pthread -lncurses
TARGET = controller
SRCS = controller.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

