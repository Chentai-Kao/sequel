appName = sequel

# default target
all: $(appName)
#	make -C ./src -f ../Makefile $(appname) # when moving Makefile to upper level

# Which compiler
CC = gcc

# Where to install
INSTDIR = ../bin

# Where are inlcude files kept
INCLUDE = .

# Options for development or release
CFLAGS = -g -Wall -D_DEBUG_ON
#CFLAGS = -O2 -Wall

# All source files
SOURCES = utils.c cmdParser.c sqlInterpreter.c
OBJECTS = $(SOURCES:.c=.o)

# local libraries (collection of object files)
#MYLIB = mylib.a

# compile and install program
$(appName): main.o $(OBJECTS)
	$(CC) $(CFLAGS) main.o $(OBJECTS) -o $@
	@if [ -d $(INSTDIR) ]; then \
		cp $(appName) $(INSTDIR);\
		chmod a+x $(INSTDIR)/$(appName);\
		chmod og-w $(INSTDIR)/$(name);\
		echo "Installed in $(INSTDIR)";\
	else \
		echo "Sorry, $(INSTDIR) does not exist";\
	fi

# object files and libraries (add dependencies here)
#$(MYLIB): $(MYLIB)(XX.o)
main.o: main.c 
$(OBJECTS): $(SOURCES)

# remove object files and libraries
clean: 
	rm -f $(OBJECTS) $(appName) tags

# tags: allow cross-file inspection
tags:
	ctags -R
