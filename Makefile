#################### Tuneables

OCHER_MAJOR?=0
OCHER_MINOR?=0
OCHER_PATCH?=7

TARGET?=posix

OCHER_EPUB=1
OCHER_TEXT=1
OCHER_HTML=1
OCHER_UI_FD?=1
#OCHER_UI_SDL
#OCHER_UI_MX50

DL_DIR=dl
BUILD_DIR=build


#################### Platforms

ifeq ($(TARGET),posix)
	CC=gcc
	CXX=g++
endif
ifeq ($(TARGET),cygwin)
	CC=gcc
	CXX=g++
endif
ifeq ($(TARGET),kobo)
	CC=$(PWD)/arm-2010q1/bin/arm-linux-gcc
	CXX=$(PWD)/arm-2010q1/bin/arm-linux-g++
	OCHER_UI_MX50?=1
endif


#################### CFLAGS

# Common CFLAGS applied everywhere
CFLAGS?=
ifeq ($(DEBUG),1)
	CFLAGS+=-g
else
	CFLAGS+=-Os -DNDEBUG
endif
ifeq ($(TARGET),cygwin)
	CFLAGS+=-DUSE_FILE32API  # for minizip
endif
ifeq ($(TARGET),haiku)
	CFLAGS+=-DUSE_FILE32API  # for minizip
endif
CFLAGS_COMMON:=$(CFLAGS)

# Additional CFLAGS for ocher
OCHER_CFLAGS:=-W -Wall --include=clc/config.h -DOCHER_MAJOR=$(OCHER_MAJOR) -DOCHER_MINOR=$(OCHER_MINOR) -DOCHER_PATCH=$(OCHER_PATCH)
ifeq ($(DEBUG),1)
	OCHER_CFLAGS+=-Werror
endif
OCHER_CFLAGS+=-Wno-unused  # TODO: only apply to minizip

default: ocher


#################### Fonts

FREEFONT_VER=20120503
FREEFONT_FILE=freefont-otf-$(FREEFONT_VER).tar.gz
FREEFONT_URL=http://ftp.gnu.org/gnu/freefont/$(FREEFONT_FILE)
FREEFONT_TGZ=$(DL_DIR)/$(FREEFONT_FILE)

fonts:
	[ -e $(FREEFONT_TGZ) ] || (echo -e "Please download $(FREEFONT_URL) to $(DL_DIR)" ; exit 1)
	mkdir -p $(BUILD_DIR)
	tar -zxf $(FREEFONT_TGZ) -C $(BUILD_DIR)


#################### FreeType

FREETYPE_VER=2.4.8
FREETYPE_TGZ=$(DL_DIR)/freetype-$(FREETYPE_VER).tar.gz
FREETYPE_DIR=$(BUILD_DIR)/freetype-$(FREETYPE_VER)
FREETYPE_DEFS=-I$(FREETYPE_DIR)/include
FREETYPE_LIB=$(FREETYPE_DIR)/objs/.libs/libfreetype.a

$(FREETYPE_LIB):
	mkdir -p $(BUILD_DIR)
	tar -zxf $(FREETYPE_TGZ) -C $(BUILD_DIR)
	cd $(FREETYPE_DIR) && CFLAGS="$(CFLAGS_COMMON)" CC=$(CC) ./configure --without-bzip2 --disable-shared --host i686-linux
	cd $(FREETYPE_DIR) && $(MAKE)

freetype_clean:
	cd $(FREETYPE_DIR) && $(MAKE) clean
	rm -f $(FREETYPE_LIB)

#################### Zlib

ZLIB_VER=1.2.5
ZLIB_TGZ=$(DL_DIR)/zlib-$(ZLIB_VER).tar.gz
ZLIB_DIR=$(BUILD_DIR)/zlib-$(ZLIB_VER)
ZLIB_LIB=$(ZLIB_DIR)/libz.a

$(ZLIB_LIB):
	mkdir -p $(BUILD_DIR)
	tar -zxf $(ZLIB_TGZ) -C $(BUILD_DIR)
	cd $(ZLIB_DIR) && CFLAGS="$(CFLAGS_COMMON)" CC=$(CC) ./configure --static
	cd $(ZLIB_DIR) && $(MAKE)

INCS+=-I$(ZLIB_DIR) -I$(ZLIB_DIR)/contrib/minizip
ZLIB_OBJS = \
	$(ZLIB_DIR)/contrib/minizip/unzip.o \
	$(ZLIB_DIR)/contrib/minizip/ioapi.o

zlib_clean:
	rm -f $(ZLIB_OBJS) $(ZLIB_LIB)


#################### miniXML

MXML_VER=2.7
MXML_TGZ=$(DL_DIR)/mxml-$(MXML_VER).tar.gz
MXML_DIR=$(BUILD_DIR)/mxml-$(MXML_VER)
MXML_LIB=$(MXML_DIR)/libmxml.a

$(MXML_LIB):
	mkdir -p $(BUILD_DIR)
	tar -zxf $(MXML_TGZ) -C $(BUILD_DIR)
	cd $(MXML_DIR) && CFLAGS="$(CFLAGS_COMMON)" CC=$(CC) ./configure --host i686-linux
	cd $(MXML_DIR) && $(MAKE) libmxml.a

INCS+=-I$(MXML_DIR)

mxml_clean:
	cd $(MXML_DIR) && $(MAKE) clean


#################### OcherBook

OCHER_CFLAGS+=-I. -DSINGLE_THREADED
OCHER_CFLAGS+=$(INCS) $(FREETYPE_DEFS)
ifeq ($(DEBUG),1)
	OCHER_CFLAGS+=-DCLC_LOG_LEVEL=5
else
	OCHER_CFLAGS+=-DCLC_LOG_LEVEL=2
endif
ifneq ($(TARGET),haiku)
	LD_FLAGS+=-lrt
endif

OCHER_OBJS = \
	clc/algorithm/Random.o \
	clc/crypto/MurmurHash2.o \
	clc/data/Buffer.o \
	clc/data/Hashtable.o \
	clc/data/List.o \
	clc/data/Set.o \
	clc/os/Clock.o \
	clc/os/Lock.o \
	clc/storage/File.o \
	clc/storage/Path.o \
	clc/support/Debug.o \
	clc/support/Logger.o \
	ocher/device/Device.o \
	ocher/fmt/Layout.o \
	ocher/fmt/Meta.o \
	ocher/ocher.o \
	ocher/ux/Browse.o \
	ocher/ux/Controller.o \
	ocher/ux/Renderer.o

ifeq ($(OCHER_EPUB),1)
OCHER_OBJS += \
	ocher/fmt/epub/Epub.o \
	ocher/fmt/epub/UnzipCache.o \
	ocher/fmt/epub/LayoutEpub.o \
	$(ZLIB_OBJS)
endif

ifeq ($(OCHER_TEXT),1)
OCHER_OBJS += \
	ocher/fmt/text/Text.o \
	ocher/fmt/text/LayoutText.o
endif

ifeq ($(TARGET),kobo)
	OCHER_CFLAGS += \
		-DTARGET_KOBO \
		-DTARGET_KOBOTOUCH
endif

ifeq ($(OCHER_UI_MX50),1)
	OCHER_CFLAGS += -DOCHER_UI_MX50
	OCHER_OBJS += \
		ocher/output/mx50/fb.o
endif

ifeq ($(OCHER_UI_FD),1)
	OCHER_CFLAGS += -DOCHER_UI_FD
	OCHER_OBJS += \
		ocher/ux/fd/BrowseFd.o \
		ocher/ux/fd/RenderFd.o \
		ocher/ux/fd/FactoryFd.o
endif

ifeq ($(OCHER_UI_NCURSES),1)
	OCHER_CFLAGS += -DOCHER_UI_NCURSES
	OCHER_OBJS += \
		ocher/ux/ncurses/Browse.o \
		ocher/ux/ncurses/Render.o \
		ocher/ux/ncurses/Factory.o
endif

OCHER_OBJS += \
	ocher/output/FreeType.o

#ODIR=obj
#OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

.c.o:
	$(CC) -c $(CFLAGS) $(OCHER_CFLAGS) $*.c -o $@

.cpp.o:
	$(CXX) -c $(CFLAGS) $(OCHER_CFLAGS) $*.cpp -o $@

ocher: $(BUILD_DIR)/ocher
$(BUILD_DIR)/ocher: $(ZLIB_LIB) $(FREETYPE_LIB) $(MXML_LIB) $(OCHER_OBJS)
	$(CXX) $(LD_FLAGS) $(CFLAGS) $(OCHER_CFLAGS) -o $@ $(OCHER_OBJS) $(ZLIB_LIB) $(FREETYPE_LIB) $(MXML_LIB)

clean: zlib_clean freetype_clean mxml_clean
	rm -f $(OCHER_OBJS) $(BUILD_DIR)/ocher

unittestpp:

test: unittestpp ocher
	# TODO

dist: ocher
	tar -C build -Jcf ocher-`uname -s`-$(OCHER_MAJOR).$(OCHER_MINOR).$(OCHER_PATCH).tar.xz ocher

dist-src: clean
	git status clc dl doc ocher
	tar -Jcf ocher-src-$(OCHER_MAJOR).$(OCHER_MINOR).$(OCHER_PATCH).tar.xz Makefile README clc dl doc ocher

.PHONY: doc

doc:
	cd ocher && doxygen ../doc/Doxyfile

help:
	@echo "Environment variables:"
	@echo ""
	@echo "Configuration"
	@echo "*	DEBUG=0		Release build"
	@echo "	DEBUG=1		Enables logging, asserts, etc"
	@echo "Target device or operating system"
	@echo "*	TARGET=posix	Linux, BSD, etc"
	@echo "	TARGET=cygwin"
	@echo "	TARGET=haiku"
	@echo "	TARGET=kobo"
	@echo ""
	@echo "Targets:"
	@echo "	clean	Clean"
	@echo "	fonts	Download GPL fonts"
	@echo "*	ocher	Build the e-reader software"
	@echo "	test	Unit tests"
	@echo "	doc	Run Doxygen"
	@echo "	dist	Build distribution packages"



