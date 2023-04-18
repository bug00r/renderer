ARFLAGS?=rcs
PATHSEP?=/
BUILDROOT?=build

BUILDDIR?=$(BUILDROOT)$(PATHSEP)$(CC)
BUILDPATH?=$(BUILDDIR)$(PATHSEP)

ifndef PREFIX
	INSTALL_ROOT=$(BUILDPATH)
else
	INSTALL_ROOT=$(PREFIX)$(PATHSEP)
	ifeq ($(INSTALL_ROOT),/)
	INSTALL_ROOT=$(BUILDPATH)
	endif
endif

ifdef DEBUG
	CFLAGS+=-ggdb
	ifeq ($(DEBUG),)
	CFLAGS+=-Ddebug=1
	else 
	CFLAGS+=-Ddebug=$(DEBUG)
	endif
endif

ifeq ($(M32),1)
	CFLAGS+=-m32
	BIT_SUFFIX+=32
endif

CFLAGS+=-std=c11 -Wpedantic -pedantic-errors -Wall -Wextra
#-ggdb
#-pg for profiling 

LDFLAGS+=-L$(BUILDDIR) -L/c/dev/lib$(BIT_SUFFIX)
CFLAGS+=-I./include -I/c/dev/include 

NAME=renderer
LIBNAME=lib$(NAME).a
LIB=$(BUILDPATH)$(LIBNAME)
OBJS=$(BUILDPATH)$(NAME).o $(BUILDPATH)camera.o

TESTBIN=$(BUILDPATH)test_$(NAME).exe
LDFLAGS+=-l$(NAME) -lscene -lr_font -lmesh -lshape -ltexture -lnoise -lfractals -lgeometry -lcrgb_array -ldl_list -lfarray -larray -lcolor -lstatistics -lutilsmath -lmat -lvec

all: mkbuilddir $(LIB)

$(LIB): $(OBJS) 
	$(AR) $(ARFLAGS) $@ $^

$(OBJS): include/$(NAME).h include/camera.h
	$(CC) $(CFLAGS) -c src/$(@F:.o=.c) -o $@ 
	
$(TESTBIN): $(LIB) test/font_provider_default.h
	$(CC) $(CFLAGS) test/$(@F:.exe=.c) test/font_provider_default.c -o $@ $(LDFLAGS) 

.PHONY: clean mkbuilddir test

test: mkbuilddir $(TESTBIN)
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
	cp $(LIB) $(INSTALL_ROOT)lib$(BIT_SUFFIX)$(PATHSEP)$(LIBNAME)