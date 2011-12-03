PROGRAM=circular-main-menu
OBJS = \
	src/main.o \
	src/cmmcircularmainmenu.o
BIN_PATH=/usr/local/bin	
GROUP=users
INCLUDES=/opt/gnome/include

CC=gcc
CFLAGS=-g -O2 -DDEBUG -I$(INCLUDES)
#CFLAGS+=-fdump-rtl-expand # callgraph generation
CFLAGS+=`pkg-config --cflags gtk+-3.0 gio-2.0 gnome-vfs-2.0`
CFLAGS+=-DGTK_DISABLE_SINGLE_INCLUDES
CFLAGS+=-DGDK_DISABLE_DEPRECATED
CFLAGS+=-DGSEAL_ENABLE

LIBS=`pkg-config --libs gtk+-3.0 gnome-vfs-2.0` -lgnome-menu

IMAGES = \
	pixmaps/close-menu-normal.svg \
	pixmaps/close-menu-prelight.svg

all: clean $(OBJS)
	$(CC) $(DEFINES) $(CFLAGS) $(OBJS) -o $(PROGRAM) $(LIBS)

clean: 
	rm -f src/*.o $(PROGRAM)

install:
	install -D -m0755 $(PROGRAM) $(DESTDIR)/usr/bin/$(PROGRAM)

uninstall:
	rm -r $(DESTDIR)/usr/bin/$(PROGRAM)

circularmainmenu:	$(OBJS) 

gdk_pixbuf_csource=gdk-pixbuf-csource

cmmstockpixbufs.h: 
	$(gdk_pixbuf_csource) --raw --build-list $(IMAGES) > src/cmmstockpixbufs.h
#        $(gdk_pixbuf_csource) --raw --build-list $(IMAGES) > src/cmmclose-menu-normal.h
#        $(gdk_pixbuf_csource) --raw --build-list $(IMAGES) > src/cmmclose-menu-prelight.h


rpm: dist
	rpmbuild -ta $(distdir).tar.gz

tar:
	tar -pczf $(distdir).tar.gz ./circular-main-menu
