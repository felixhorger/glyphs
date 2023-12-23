.PHONY: all clean

build/glyphs.o: src/*.c
	gcc -fPIC -I/usr/include/freetype2 -c src/glyphs.c -o build/glyphs.o
#

bin/glyphs: build/glyphs.o ../glaze/lib/libglaze.a
	#gcc -L../glaze/lib -I../glaze/lib -o bin/glyphs build/glyphs.o -lglaze -ldl -lm -lGL -lglfw
	gcc -o bin/glyphs build/glyphs.o ../glaze/lib/libglaze.a -ldl -lm -lGL -lfreetype -lglfw
#

# TODO: remote glfw

all: bin/glyphs

clean:
	rm -f bin/*
	rm -f build/*
