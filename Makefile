#!/bin/make
CC=gcc
CLAGS=-Wall -pipe -O2
TARGET=smokenrg
OBJECTS=smokenrg-1.0.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	$(RM) $(TARGET) $(OBJECTS)
