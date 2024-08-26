# nmake makefile
#
# Tools used:
#  Compile::Watcom Resource Compiler
#  Compile::GNU C
#  Make: nmake or GNU make
all : xtime.exe

xtime.exe : xtime.obj xtime.def
	gcc -Zomf xtime.obj xtime.def -o xtime.exe

xtime.obj : xtime.c xtime.h
	gcc -Wall -Zomf -c -O2 xtime.c -o xtime.obj

clean :
	rm -rf *exe *res *obj *dll *map
