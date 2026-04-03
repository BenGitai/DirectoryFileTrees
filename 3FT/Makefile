#--------------------------------------------------------------------
# Makefile for Assignment 4, Part 3
# Author: Benjamin Gitai, Jeremy Arking
#--------------------------------------------------------------------

all: ft

clean:
	rm -f ft meminfo*.out

clobber: clean
	rm -f dynarray.o path.o fileFT.o dirFT.o ft.o

ft: ft_client.o ft.o dynarray.o path.o
	gcc217 -g ft_client.o ft.o dynarray.o path.o -o ft

ft_client.o: ft_client.c ft.h a4def.h
	gcc217 -g -c ft_client.c

ft.o: ft.c ft.h a4def.h dynarray.h path.h
	gcc217 -g -c ft.c

dynarray.o: dynarray.c dynarray.h
	gcc217 -g -c dynarray.c

path.o: path.c path.h
	gcc217 -g -c path.c

fileFT.o: fileFT.c fileFT.h a4def.h dynarray.h path.h
	gcc217 -g -c fileFT.c

dirFT.o: dirFT.c dirFT.h a4def.h dynarray.h path.h
	gcc217 -g -c dirFT.c