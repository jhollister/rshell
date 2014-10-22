CC=g++
CFLAGS=-Wall -Werror -ansi -pedantic -g 
SOURCES=$(wildcard src/*.cpp)
EDIR=bin
TARGET=rshell

all: $(TARGET)

$(TARGET): 
	mkdir -p $(EDIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EDIR)/$@
clean:
	rm -rf $(EDIR)
