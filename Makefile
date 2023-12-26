.PHONY: all test clean

build/glyphs.o: src/glyphs.c
	gcc -fPIC -I/usr/include/freetype2 -c src/glyphs.c -o build/glyphs.o
#

lib/libglyphs.so: build/glyphs.o ../glaze/lib/libglaze.so
	gcc -L../glaze/lib -I../glaze/lib -fPIC -shared -o $@ build/glyphs.o -ldl -lGL -lglaze -lfreetype
#

lib/libglyphs.a: build/glyphs.o ../glaze/lib/libglaze.a
	ar rcs $@ build/glyphs.o ../glaze/lib/libglaze.a
#

test/test: test/test.c lib/libglyphs.a ../glaze/lib/libglaze.a
	gcc -o test/test.o -c test/test.c
	gcc -o test/test test/test.o lib/libglyphs.a ../glaze/lib/libglaze.a -ldl -lm -lGL -lfreetype -lglfw
#


all: lib/libglyphs.so lib/libglyphs.a

test: test/test

clean:
	rm -f lib/*
	rm -f build/*
#

