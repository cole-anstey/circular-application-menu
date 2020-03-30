PROGRAM=circular-application-menu
OBJS = \
	src/main.o \
	src/cmmcircularmainmenu.o
BIN_PATH=/usr/local/bin	
GROUP=users
INCLUDES=/opt/gnome/include
CC=gcc
CFLAGS=-g -O2 -DDEBUG -DXDG_CONFIG_DIRS='"$(XDG_CONFIG_DIRS)"' -DXDG_MENU_PREFIX='"$(XDG_MENU_PREFIX)"' -I$(INCLUDES)
CFLAGS+=`pkg-config --cflags gtk+-3.0 gio-unix-2.0 libgnome-menu-3.0`
CFLAGS+=-DGTK_DISABLE_SINGLE_INCLUDES
CFLAGS+=-DGDK_DISABLE_DEPRECATED
CFLAGS+=-DGSEAL_ENABLE

LIBS=`pkg-config --libs gtk+-3.0  gio-unix-2.0 libgnome-menu-3.0` -lX11 -lm

# XPMs do not support transparency only image masks.
IMAGES_CONVERT= \
	pixmaps/close-menu-normal.svg \
	pixmaps/open-sub-menu-normal.svg \
	pixmaps/reflection.svg \
	pixmaps/close-menu-prelight.svg \
	pixmaps/open-sub-menu-prelight.svg \
	pixmaps/debian-emblem-normal.svg
IMAGE_CONVERSION=mogrify -background none -format png --

IMAGES_INSTALL= \
	pixmaps/gnome-emblem-normal.svg \
	pixmaps/ubuntu-emblem-prelight.svg \
	pixmaps/debian-emblem-prelight.svg \
	pixmaps/gnome-emblem-prelight.svg \
	pixmaps/ubuntu-emblem-normal.svg

all: clean cmmstockpixbufs.c $(OBJS)
#all: clean $(OBJS)
	$(CC) $(DEFINES) $(CFLAGS) $(OBJS) -o $(PROGRAM) $(LIBS)

clean:
	rm -f src/*.o $(PROGRAM)
	rm -f pixmaps/*.png
	rm -f src/cmmstockpixbufs.*c

install:
	install -D -m0755 $(PROGRAM) $(DESTDIR)/usr/bin/$(PROGRAM)	
	install -D -m0755 $(PROGRAM:=.desktop) $(DESTDIR)/usr/share/applications/
	install -v -m0755 -d $(DESTDIR)/usr/share/$(PROGRAM)/pixmaps/
	install -v -m0755 $(PROGRAM:=.png) -t $(DESTDIR)/usr/share/$(PROGRAM)/
	install -v -m0755 $(IMAGES_INSTALL:.svg=.png) -t $(DESTDIR)/usr/share/$(PROGRAM)/pixmaps/

uninstall:
	rm -r $(DESTDIR)/usr/bin/$(PROGRAM)
	rm -r $(DESTDIR)/usr/share/$(PROGRAM)
	rm -r $(DESTDIR)/usr/share/applications/$(PROGRAM:=.desktop)

circularmainmenu:	$(OBJS)

cmmstockpixbufs.c: 
	$(IMAGE_CONVERSION) $(IMAGES_INSTALL)
	$(IMAGE_CONVERSION) $(IMAGES_CONVERT)
	glib-compile-resources --target=src/cmmstockpixbufs.c --generate pixmaps/circular-application-menu.gresource.xml

rpm: dist
	rpmbuild -ta $(distdir).tar.gz

tar:
	tar -pczf $(distdir).tar.gz ./circular-main-menu
