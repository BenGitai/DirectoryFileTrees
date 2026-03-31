#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3
# Author: Benjamin Gitai, Jeremy Arking
#--------------------------------------------------------------------

all: filetree

clean:
	rm -f filetree meminfo*.out

clobber: clean
	rm -f dynarray.o path.o fileFT.o dirFT.o ft.o

filetree: dynarray.o path.o ft.o
	gcc217 -g ft.o dynarray.o path.o -o filetree

ft.o: fileFT.o dirFT.o ft.h a4def.h dynarray.h path.h
	gcc217 -g -c fileFT.c dirFT.c

dirFT.o: dirFT.c dirFT.h dynarray.h path.h
	gcc217 -g -c dirFT.c

fileFT.o: fileFT.c fileFT.h dynarray.h path.h
	gcc217 -g -c fileFT.c