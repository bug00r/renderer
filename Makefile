#MAKE?=mingw32-make
AR?=ar
ARFLAGS?=rcs
PATHSEP?=/
CC=gcc
BUILDROOT?=build

ifeq ($(CLANG),1)
	export CC=clang
endif

BUILDDIR?=$(BUILDROOT)$(PATHSEP)$(CC)
BUILDPATH?=$(BUILDDIR)$(PATHSEP)

INSTALL_ROOT?=$(BUILDPATH)

ifeq ($(DEBUG),1)
	export debug=-ggdb -Ddebug=1
	export isdebug=1
endif

ifeq ($(ANALYSIS),1)
	export analysis=-Danalysis=1
	export isanalysis=1
endif

ifeq ($(DEBUG),2)
	export debug=-ggdb -Ddebug=2
	export isdebug=1
endif

ifeq ($(DEBUG),3)
	export debug=-ggdb -Ddebug=3
	export isdebug=1
endif

ifeq ($(OUTPUT),1)
	export outimg= -Doutput=1
endif

BIT_SUFFIX=

ifeq ($(M32),1)
	CFLAGS+=-m32
	BIT_SUFFIX+=32
endif

CFLAGS+=-std=c11 -Wpedantic -pedantic-errors -Wall -Wextra $(debug)
#-ggdb
#-pg for profiling 

LIB?=-L/c/dev/lib$(BIT_SUFFIX)
INCLUDE?= -I/c/dev/include -I./include

LIBNAME=librenderer.a
OBJS=$(BUILDPATH)renderer.o $(BUILDPATH)camera.o $(BUILDPATH)texture_cache.o
SRC_DIR=src/
SRC_FILES=camera renderer
SRC=$(patsubst %,$(SRC_DIR)%.c,$(SRC_FILES))

INCLUDEDIR= $(INCLUDE)

TESTSRC=test/test_renderer.c
CACHETESTSRC=test/test_texture_cache.c
TESTBIN=test_renderer.exe
CACHETESTBIN=test_texture_cache.exe
LIBS=-lscene -lmesh -lshape -ltexture -lnoise -lfractals -lgeometry -lcrgb_array -ldl_list -lfarray -larray -lcolor -lstatistics -lutilsmath -lmat -lvec
TESTLIB=-lrenderer 
LIBDIR=-L$(BUILDDIR) $(LIB)

ifeq ($(isdebug),1)
	INCLUDEDIR += -I./../collections/linked_list/
	TESTLIBDIR += -L./../collections/linked_list/$(BUILDDIR)
	TESTLIB += -llinked_list
endif

all: mkbuilddir $(BUILDPATH)$(LIBNAME)

$(BUILDPATH)$(LIBNAME): $(OBJS) 
	$(AR) $(ARFLAGS) $(BUILDPATH)$(LIBNAME) $(OBJS)

$(BUILDPATH)renderer.o: src/renderer.c
	$(CC) $(CFLAGS) -c src/renderer.c -o $(BUILDPATH)renderer.o $(INCLUDEDIR)

$(BUILDPATH)camera.o: src/camera.c
	$(CC) $(CFLAGS) -c src/camera.c -o $(BUILDPATH)camera.o  $(INCLUDEDIR)
	
$(BUILDPATH)texture_cache.o: src/texture_cache.c
	$(CC) $(CFLAGS) -c src/texture_cache.c -o $(BUILDPATH)texture_cache.o  $(INCLUDEDIR)
	
$(BUILDPATH)$(TESTBIN):
	$(CC) $(CFLAGS) $(TESTSRC) -o $(BUILDPATH)$(TESTBIN) $(INCLUDEDIR) $(LIBDIR) $(LIBDIR) $(TESTLIB) $(LIBS) 

$(BUILDPATH)$(CACHETESTBIN):
	$(CC) $(CFLAGS) $(CACHETESTSRC) -o $(BUILDPATH)$(CACHETESTBIN) $(INCLUDEDIR) $(LIBDIR) $(LIBDIR) $(TESTLIB) $(LIBS) 

.PHONY: clean mkbuilddir test

test: mkbuilddir $(BUILDPATH)$(LIBNAME) $(BUILDPATH)$(TESTBIN) $(BUILDPATH)$(CACHETESTBIN)
	./$(BUILDPATH)$(TESTBIN)
	./$(BUILDPATH)$(CACHETESTBIN)

mkbuilddir:
	mkdir -p $(BUILDDIR)

clean:
	-rm -dr $(BUILDROOT)

install:
	mkdir -p $(INSTALL_ROOT)include
	mkdir -p $(INSTALL_ROOT)lib$(BIT_SUFFIX)
	cp ./include/renderer.h $(INSTALL_ROOT)include/renderer.h
	cp ./include/camera.h $(INSTALL_ROOT)include/camera.h
	cp ./include/texture_cache.h $(INSTALL_ROOT)include/texture_cache.h
	cp $(BUILDPATH)$(LIBNAME) $(INSTALL_ROOT)lib$(BIT_SUFFIX)/$(LIBNAME)