CC=g++
CFLAGS=-Wall -Werror -ansi -pedantic -g
RSHELL_SOURCES=src/rshell.cpp
LS_SOURCES=src/ls.cpp
EDIR=bin
RSHELL_TARGET=rshell
LS_TARGET=ls

all: $(RSHELL_TARGET) $(LS_TARGET)
$(RSHELL_TARGET):
	mkdir -p $(EDIR)
	$(CC) $(CFLAGS) $(RSHELL_SOURCES) -o $(EDIR)/$@
$(LS_TARGET):
	mkdir -p $(EDIR)
	$(CC) $(CFLAGS) $(LS_SOURCES) -o $(EDIR)/$@
clean:
	rm -rf $(EDIR)
