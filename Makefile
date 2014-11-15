CC=g++
CFLAGS=-Wall -Werror -ansi -pedantic -std=c++11 -g
RSHELL_SOURCES=src/rshell.cpp
LS_SOURCES=src/ls.cpp
CP_SOURCES=src/cp.cpp
EDIR=bin
RSHELL_TARGET=rshell
LS_TARGET=ls
CP_TARGET=cp

all: $(RSHELL_TARGET) $(LS_TARGET) $(CP_TARGET)
$(RSHELL_TARGET):
	mkdir -p $(EDIR)
	$(CC) $(CFLAGS) $(RSHELL_SOURCES) -o $(EDIR)/$@
$(LS_TARGET):
	mkdir -p $(EDIR)
	$(CC) $(CFLAGS) $(LS_SOURCES) -o $(EDIR)/$@
$(CP_TARGET):
	mkdir -p $(EDIR)
	$(CC) $(CFLAGS) $(CP_SOURCES) -o $(EDIR)/$@
clean:
	rm -rf $(EDIR)
