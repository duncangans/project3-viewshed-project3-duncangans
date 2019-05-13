PLATFORM = $(shell uname)

#######################
## Compilation flags ##
#######################
# Debugging
#CFLAGS = -g3# -pg
# Release
CFLAGS = -O3 -DNDEBUG
LDFLAGS=

CFLAGS+= -Wall -Wstrict-prototypes


ifeq ($(PLATFORM),Darwin)
## Mac OS X
CFLAGS += -m64 -isystem/usr/local/include
LDFLAGS+= -m64 -lc -framework AGL -framework OpenGL -framework GLUT \
	-framework Foundation
else
## Linux
CFLAGS += -m64
INCLUDEPATH  = -I/usr/include/GL/ 
LIBPATH = -L/usr/lib64 -L/usr/X11R6/lib
LDFLAGS+=  -lGL -lglut -lrt -lGLU -lX11 -lm  -lXmu -lXext -lXi
endif


CC = gcc 
MODULES = llist.o grid.o utils.o gmath.o colorizer.o rtimer.o 
GRAPHICS = $(LIBPATH) $(LDFLAGS) 
BINARIES = grid_info grid_diff grid_simp  render2d render3d 

default: $(BINARIES) 

grid_info: modules grid_info.o
	$(CC)  -lm $(MODULES) grid_info.o -o grid_info

grid_diff: modules grid_diff.o
	$(CC) -lm $(MODULES) grid_diff.o -o grid_diff

grid_simp: modules grid_simp.o
	$(CC) -lm $(MODULES) grid_simp.o -o grid_simp

render2d: modules render.o render2d.o
	$(CC) -lm $(GRAPHICS) $(MODULES) render.o render2d.o -o render2d

render3d: modules render.o render3d.o
	$(CC) -lm $(GRAPHICS) $(MODULES) render.o render3d.o -o render3d

modules: llist.o  grid.o utils.o gmath.o colorizer.o rtimer.o 


%.o: %.c
	$(CC) $(INCLUDEPATH) -c $< -o $@

clean:
	$(RM) *.o $(BINARIES)
