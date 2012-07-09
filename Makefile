#################### Tuneables

default: ocher

TARGET?=kobo

help:
	@echo "Environment variables:"
	@echo ""
	@echo "Debug"
	@echo "*	DEBUG=0		Release build"
	@echo "	DEBUG=1		Enables logging, asserts, etc"
	@echo "Target device"
	@echo "*	TARGET=native	Compile natively for testing"
	@echo "	TARGET=kobo	Compile for KoboTouch"
	@echo ""
	@echo "Targets:"
	@echo "*	ocher"
	@echo "	doc"
	@echo "	clean"
	@echo "	test"



#################### Common settings

TARGET?=native

# Common CFLAGS applied everywhere
ifeq ($(DEBUG),1)
	CFLAGS?=-g
else
	CFLAGS?=-Os -DNDEBUG
endif
CFLAGS_COMMON:=$(CFLAGS)

# Additional CFLAGS for ocher (more picky than 3rd party libs)
OCHER_CFLAGS?=-W -Wall

DL_DIR=dl
BUILD_DIR=build

ifeq ($(TARGET),native)
	CC?=gcc
	CXX?=g++
	OCHER_UI_FD=1
else
	CC?=`pwd`/arm-2010q1/bin/arm-linux-gcc
	CXX?=`pwd`/arm-2010q1/bin/arm-linux-g++
	OCHER_UI_MX50=1
endif


#################### FreeType

FREETYPE_VER=2.4.8
FREETYPE_TGZ=$(DL_DIR)/freetype-$(FREETYPE_VER).tar.gz
FREETYPE_DIR=$(BUILD_DIR)/freetype-$(FREETYPE_VER)
FREETYPE_DEFS=-I$(FREETYPE_DIR)
FREETYPE_LIB=$(FREETYPE_DIR)/objs/.libs/libfreetype.a

$(FREETYPE_LIB):
	mkdir -p $(BUILD_DIR)
	tar -zxf $(FREETYPE_TGZ) -C $(BUILD_DIR)
	cd $(FREETYPE_DIR) && CFLAGS="$(CFLAGS_COMMON)" CC=$(CC) ./configure --without-bzip2 --disable-shared
	cd $(FREETYPE_DIR) && $(MAKE)


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


#################### miniXML

MXML_VER=2.7
MXML_TGZ=$(DL_DIR)/mxml-$(MXML_VER).tar.gz
MXML_DIR=$(BUILD_DIR)/mxml-$(MXML_VER)
MXML_LIB=$(MXML_DIR)/libmxml.a

$(MXML_LIB):
	mkdir -p $(BUILD_DIR)
	tar -zxf $(MXML_TGZ) -C $(BUILD_DIR)
	cd $(MXML_DIR) && CFLAGS="$(CFLAGS_COMMON)" CC=$(CC) ./configure
	cd $(MXML_DIR) && make

INCS+=-I$(MXML_DIR)


#################### OcherBook

CFLAGS=-I. $(INCS) $(OCHER_CFLAGS) -DSINGLE_THREADED
ifeq ($(DEBUG),1)
	CFLAGS+=-DMT_LOG_LEVEL=5
else
	CFLAGS+=-DMT_LOG_LEVEL=2
endif
CFLAGS+=$(CFLAGS_COMMON)
LD_FLAGS+=-lrt

OCHER_OBJS = \
	clc/crypto/MurmurHash2.o \
	clc/data/Buffer.o \
	clc/data/Hashtable.o \
	clc/data/List.o \
	clc/data/Set.o \
	clc/os/Clock.o \
	clc/storage/Path.o \
	clc/support/Debug.o \
	clc/support/Logger.o \
	ocher/device/Device.o \
	ocher/fmt/epub/Epub.o \
	ocher/fmt/epub/UnzipCache.o \
	ocher/fmt/epub/LayoutEpub.o \
	ocher/fmt/text/Text.o \
	ocher/fmt/text/LayoutText.o \
	ocher/fmt/Meta.o \
	ocher/layout/Layout.o \
	ocher/main.o \
	ocher/ui/Browse.o \
	ocher/ui/Controller.o \
	ocher/ui/Renderer.o \
	$(ZLIB_OBJS)

ifeq ($(TARGET),KoboTouch)
	CFLAGS += \
		-DTARGET_KOBO \
		-DTARGET_KOBOTOUCH
endif

ifeq ($(OCHER_UI_MX50),1)
	CFLAGS += -DOCHER_UI_MX50
	OCHER_OBJS += \
		ocher/fb/mx50/fb.o
endif

ifeq ($(OCHER_UI_FD),1)
	CFLAGS += -DOCHER_UI_FD
	OCHER_OBJS += \
		ocher/ui/fd/BrowseFd.o \
		ocher/ui/fd/RenderFd.o \
		ocher/ui/fd/FactoryFd.o
endif

ifeq ($(OCHER_UI_NCURSES),1)
	CFLAGS += -DOCHER_UI_NCURSES
	OCHER_OBJS += \
		ocher/ui/ncurses/Browse.o \
		ocher/ui/ncurses/Render.o \
		ocher/ui/ncurses/Factory.o
endif

#ODIR=obj
#OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

#.c.o:
#	$(CC) -c $(CFLAGS) -Wno-unused-parameter $*.c -o $@

.cpp.o:
	$(CXX) -c $(CFLAGS) $*.cpp -o $@

ocher: $(BUILD_DIR)/ocher
$(BUILD_DIR)/ocher: $(ZLIB_LIB) $(FREETYPE_LIB) $(MXML_LIB) $(OCHER_OBJS)
	$(CXX) $(LD_FLAGS) $(CFLAGS) -o $@ $(OCHER_OBJS) $(ZLIB_LIB) $(FREETYPE_LIB) $(MXML_LIB)

clean:
	/bin/rm -f $(OCHER_OBJS) $(BUILD_DIR)/ocher

test: ocher
	# TODO

.PHONY: doc

doc:
	cd ocher && doxygen ../doc/Doxyfile
