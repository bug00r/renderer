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

LIBSDIR?=-L/c/dev/lib$(BIT_SUFFIX)
INCLUDE?= -I/c/dev/include -I./include

NAME=renderer
LIBNAME=lib$(NAME).a
LIB=$(BUILDPATH)$(LIBNAME)
OBJS=$(BUILDPATH)$(NAME).o $(BUILDPATH)camera.o $(BUILDPATH)rasterizer.o

INCLUDEDIR= $(INCLUDE)

TESTBIN=$(BUILDPATH)test_$(NAME).exe
LIBS=-l$(NAME) -lscene -lr_font -lmesh -lshape -ltexture -lnoise -lfractals -lgeometry -lcrgb_array -ldl_list -lfarray -larray -lcolor -lstatistics -lutilsmath -lmat -lvec
LIBDIR=-L$(BUILDDIR) $(LIBSDIR)

all: mkbuilddir $(LIB) $(TESTBIN)

$(LIB): $(OBJS) 
	$(AR) $(ARFLAGS) $@ $^

$(OBJS): include/$(NAME).h include/camera.h include/rasterizer.h
	$(CC) $(CFLAGS) -c src/$(@F:.o=.c) -o $@ $(INCLUDEDIR)
	
$(TESTBIN): $(LIB) test/font_provider_default.h
	$(CC) $(CFLAGS) test/$(@F:.exe=.c) test/font_provider_default.c -o $@ $(INCLUDEDIR) $(LIBDIR) $(LIBS) 

.PHONY: clean mkbuilddir test

test:
	./$(TESTBIN)

mkbuilddir:
	mkdir -p $(BUILDDIR)

clean:
	-rm -dr $(BUILDROOT)

install:
	mkdir -p $(INSTALL_ROOT)include
	mkdir -p $(INSTALL_ROOT)lib$(BIT_SUFFIX)
	cp ./include/$(NAME).h $(INSTALL_ROOT)include$(PATHSEP)$(NAME).h
	cp ./include/camera.h $(INSTALL_ROOT)include$(PATHSEP)camera.h
	cp ./include/rasterizer.h $(INSTALL_ROOT)include$(PATHSEP)rasterizer.h
	cp $(LIB) $(INSTALL_ROOT)lib$(BIT_SUFFIX)$(PATHSEP)$(LIBNAME)