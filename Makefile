# Makefile for building the DP2 Indav project with libusb
CC = gcc
CFLAGS = -Wall -g -lusb-1.0
LDFLAGS = -lusb-1.0

TARGET = dp2indav

all: $(TARGET)

$(TARGET): $(TARGET).o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(TARGET) *.o