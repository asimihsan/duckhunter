NAME=duckhunter
VERSION=0.0.1

all: duckhunter

duckhunter: ./build/makefiles/Makefile
	@make -C ./build/makefiles

deps: libzmq libbstring libglib

./build/makefiles/Makefile:
	@gyp duckhunter.gyp --depth=. -f make \
		--generator-output=./build/makefiles -Dlibrary=static_library

./external/libzmq/Makefile.in: 
	cd external/libzmq; ./autogen.sh

./external/libzmq/config.status: ./external/libzmq/Makefile.in
	cd external/libzmq; ./configure --enable-static --disable-shared --with-pic

libzmq: ./external/libzmq/config.status
	cd external/libzmq; make

./external/bstring/Makefile.in:
	cd external/bstring; autoreconf -i

./external/bstring/config.status: ./external/bstring/Makefile.in
	cd external/bstring; ./configure --enable-static --disable-shared \
		--with-pic

libbstring: ./external/bstring/config.status
	cd external/bstring; make

./external/glib/Makefile.in:
	cd external/glib; ./autogen.sh

./external/glib/config.status: ./external/glib/Makefile.in
	cd external/glib; ./configure --enable-static --disable-shared --with-pic

libglib: ./external/glib/config.status
	cd external/glib; make

test: duckhunter
	@valgrind --track-origins=yes --leak-check=full --dsymutil=yes \
		build/makefiles/out/Default/duckhunter

clean:
	rm -rf ./build	

distclean: clean
	-cd external/libzmq; make distclean
	cd external/libzmq; rm -f Makefile.in
	-cd external/bstring; make distclean
	cd external/bstring; rm -f Makefile.in

package-deps: 
	if [ ! -d packages ]; then \
		mkdir packages; \
	fi; \
	if [ -d /tmp/installdir ]; then \
		rm -rf /tmp/installdir; \
	fi; \
	mkdir -p /tmp/installdir/usr/local/bin; \
	cp build/makefiles/out/Default/duckhunter /tmp/installdir/usr/local/bin/duckhunter

rpm: duckhunter package-deps
	fpm -s dir -t rpm -n $(NAME) -v $(VERSION) -C /tmp/installdir \
		--package packages/duckhunter-VERSION_ARCH.rpm \
		--depends "libstdc++" \
		--force \
		usr

deb: duckhunter package-deps
	fpm -s dir -t deb -n $(NAME) -v $(VERSION) -C /tmp/installdir \
		--package packages/duckhunter-VERSION_ARCH.deb \
		--depends "libstdc++" \
		--force \
		usr
