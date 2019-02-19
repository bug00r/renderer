include ../make_config

LIBNAME=librenderer.a
OBJS=$(BUILDPATH)renderer.o $(BUILDPATH)camera.o
SRC_DIR=src/
SRC_FILES=camera renderer
SRC=$(patsubst %,$(SRC_DIR)%.c,$(SRC_FILES))

INCLUDEDIR= -I./../math/algorithm/fractals/ \
			-I./../math/algorithm/noise/ \
			-I./../math/statistics \
			-I./../math/vec \
			-I./../math/mat \
			-I./../math/utils \
			-I./../collections/array \
			-I./../color \
			-I./../texture \
			-I./../shape \
			-I./../mesh \
			-I./../scene \
			-I./include

TESTSRC=test/test_renderer.c
TESTBIN=test_renderer.exe
LIB=-lscene -lmesh -lshape -ltexture -lnoise -lfractals -lcrgb_array -lfarray -larray -lcolor -lstatistics -lutilsmath -lmat -lvec
TESTLIB=-lrenderer 
LIBDIR=-L$(BUILDDIR) \
		   -L./../math/algorithm/fractals/$(BUILDDIR) \
		   -L./../math/algorithm/noise/$(BUILDDIR) \
		   -L./../scene/$(BUILDDIR) \
		   -L./../mesh/$(BUILDDIR) \
		   -L./../shape/$(BUILDDIR) \
		   -L./../color/$(BUILDDIR) \
		   -L./../texture/$(BUILDDIR) \
		   -L./../collections/array/$(BUILDDIR) \
		   -L./../math/statistics/$(BUILDDIR) \
		   -L./../math/utils/$(BUILDDIR) \
		   -L./../math/mat/$(BUILDDIR) \
		   -L./../math/vec/$(BUILDDIR) 

ifeq ($(isdebug),1)
	INCLUDEDIR += -I./../collections/linked_list/
	TESTLIBDIR += -L./../collections/linked_list/$(BUILDDIR)
	TESTLIB += -llinked_list
endif

ifeq ($(PROFILING),1)
	CFLAGS=-std=c11 -pg -Wpedantic -pedantic-errors -Wall -Wextra
endif

CFLAGS=-std=c11 -Wpedantic -pedantic-errors -Wall -Wextra $(debug)

all: mkbuilddir $(BUILDPATH)$(LIBNAME)

$(BUILDPATH)$(LIBNAME): $(OBJS) 
	$(AR) $(ARFLAGS) $(BUILDPATH)$(LIBNAME) $(OBJS)

$(BUILDPATH)renderer.o: src/renderer.c
	$(CC) $(CFLAGS) -c src/renderer.c -o $(BUILDPATH)renderer.o $(INCLUDEDIR)

$(BUILDPATH)camera.o: src/camera.c
	$(CC) $(CFLAGS) -c src/camera.c -o $(BUILDPATH)camera.o  $(INCLUDEDIR)
	
$(BUILDPATH)$(TESTBIN):
	$(CC) $(CFLAGS) $(TESTSRC) -o $(BUILDPATH)$(TESTBIN) $(INCLUDEDIR) $(LIBDIR) $(LIBDIR) $(LIB) $(TESTLIB)
	
.PHONY: clean mkbuilddir test

test: mkbuilddir $(BUILDPATH)$(LIBNAME) $(BUILDPATH)$(TESTBIN)
	./$(BUILDPATH)$(TESTBIN)

mkbuilddir:
	mkdir -p $(BUILDDIR)

clean:
	-rm -dr $(BUILDROOT)
	