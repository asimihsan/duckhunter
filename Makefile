all: duckhunter

duckhunter: ./build/makefiles/Makefile
	@make -C ./build/makefiles

deps: libzmq libbstring libglib

./build/makefiles/Makefile:
	@gyp duckhunter.gyp --depth=. -f make --generator-output=./build/makefiles -Dlibrary=static_library

./external/libzmq/Makefile.in: 
	cd external/libzmq; ./autogen.sh

./external/libzmq/config.status: ./external/libzmq/Makefile.in
	cd external/libzmq; ./configure --enable-static --disable-shared --with-pic

libzmq: ./external/libzmq/config.status
	cd external/libzmq; make

./external/bstring/Makefile.in:
	cd external/bstring; autoreconf -i

./external/bstring/config.status: ./external/bstring/Makefile.in
	cd external/bstring; ./configure --enable-static --disable-shared --with-pic

libbstring: ./external/bstring/config.status
	cd external/bstring; make

./external/glib/Makefile.in:
	cd external/glib; ./autogen.sh

./external/glib/config.status: ./external/glib/Makefile.in
	cd external/glib; ./configure --enable-static --disable-shared --with-pic

libglib: ./external/glib/config.status
	cd external/glib; make

test: duckhunter
	@valgrind --track-origins=yes --leak-check=full --show-reachable=yes build/makefiles/out/Default/duckhunter

clean:
	rm -rf ./build	

distclean: clean
	-cd external/libzmq; make distclean
	cd external/libzmq; rm -f Makefile.in
	-cd external/bstring; make distclean
	cd external/bstring; rm -f Makefile.in
