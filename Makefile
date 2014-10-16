CC=g++
CFLAGS=-Wall -Werror -ansi -pedantic 
SOURCES=$(wildcard src/*.cpp)
EDIR=bin
TARGET=rshell

all: $(TARGET)

$(TARGET): 
	mkdir -p $(EDIR)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EDIR)/$@
clean:
	rm -rf $(EDIR)
