.PHONY: clean config test dist dist-src doc help

# TODO:
# conditionalize freetype, etc
# header dependencies
# move all objs to build/
# obj-$(CONFIG_FOO) += foo.o
# auto-detect platform
# create config.h


#################### Tuneables

DL_DIR=dl
BUILD_DIR=build

include ocher.config


#################### Platforms

ifeq ($(OCHER_TARGET),posix)
	CC=gcc
	CXX=g++
endif
ifeq ($(OCHER_TARGET),cygwin)
	CC=gcc
	CXX=g++
endif
ifeq ($(OCHER_TARGET),kobo)
	CC=$(PWD)/arm-2010q1/bin/arm-linux-gcc
	CXX=$(PWD)/arm-2010q1/bin/arm-linux-g++
endif
ifeq ($(OCHER_VERBOSE),1)
	QUIET=
	MSG=@true
else
	QUIET=@
	MSG=@echo
endif


#################### CFLAGS

# Common CFLAGS applied everywhere
CFLAGS?=
ifeq ($(OCHER_DEBUG),1)
	CFLAGS+=-g
else
	CFLAGS+=-Os -DNDEBUG
endif
ifeq ($(OCHER_TARGET),cygwin)
	CFLAGS+=-DUSE_FILE32API  # for minizip
	INCS+=-I/usr/include/ncurses
endif
ifeq ($(OCHER_TARGET),haiku)
	CFLAGS+=-DUSE_FILE32API  # for minizip
endif
CFLAGS_COMMON:=$(CFLAGS)

# Additional CFLAGS for ocher
OCHER_CFLAGS:=-W -Wall -Wextra -Wformat=2 --include=clc/config.h -DOCHER_MAJOR=$(OCHER_MAJOR) -DOCHER_MINOR=$(OCHER_MINOR) -DOCHER_PATCH=$(OCHER_PATCH)
ifeq ($(OCHER_DEV),1)
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
	@[ -e $(FREEFONT_TGZ) ] || (echo -e "Please download $(FREEFONT_URL) to $(DL_DIR)" ; exit 1)
	@mkdir -p $(BUILD_DIR)
	tar -zxf $(FREEFONT_TGZ) -C $(BUILD_DIR)


#################### FreeType

FREETYPE_VER=2.4.8
FREETYPE_TGZ=$(DL_DIR)/freetype-$(FREETYPE_VER).tar.gz
FREETYPE_DIR=$(BUILD_DIR)/freetype-$(FREETYPE_VER)
FREETYPE_DEFS=-I$(FREETYPE_DIR)/include
FREETYPE_LIB=$(FREETYPE_DIR)/objs/.libs/libfreetype.a

$(FREETYPE_LIB):
	@mkdir -p $(BUILD_DIR)
	tar -zxf $(FREETYPE_TGZ) -C $(BUILD_DIR)
	cd $(FREETYPE_DIR) && CFLAGS="$(CFLAGS_COMMON)" CC=$(CC) ./configure --without-bzip2 --disable-shared --host i686-linux
	cd $(FREETYPE_DIR) && $(MAKE)

freetype_clean:
	cd $(FREETYPE_DIR) && $(MAKE) clean || true
	rm -f $(FREETYPE_LIB)

#################### Zlib

ZLIB_VER=1.2.5
ZLIB_TGZ=$(DL_DIR)/zlib-$(ZLIB_VER).tar.gz
ZLIB_DIR=$(BUILD_DIR)/zlib-$(ZLIB_VER)
ZLIB_LIB=$(ZLIB_DIR)/libz.a

$(ZLIB_LIB):
	@mkdir -p $(BUILD_DIR)
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
	@mkdir -p $(BUILD_DIR)
	tar -zxf $(MXML_TGZ) -C $(BUILD_DIR)
	cd $(MXML_DIR) && CFLAGS="$(CFLAGS_COMMON)" CC=$(CC) ./configure --host i686-linux
	cd $(MXML_DIR) && $(MAKE) libmxml.a

INCS+=-I$(MXML_DIR)

mxml_clean:
	cd $(MXML_DIR) && $(MAKE) clean || true


#################### OcherBook

OCHER_CFLAGS+=-I. -Ibuild -DSINGLE_THREADED
OCHER_CFLAGS+=$(INCS) $(FREETYPE_DEFS)
ifeq ($(OCHER_DEBUG),1)
	OCHER_CFLAGS+=-DCLC_LOG_LEVEL=5
else
	OCHER_CFLAGS+=-DCLC_LOG_LEVEL=2
endif
ifneq ($(OCHER_TARGET),haiku)
	LD_FLAGS+=-lrt
endif
LD_FLAGS+=$(OCHER_LIBS)

OCHER_OBJS = \
	clc/algorithm/Random.o \
	clc/crypto/MurmurHash2.o \
	clc/data/Buffer.o \
	clc/data/Hashtable.o \
	clc/data/List.o \
	clc/data/Set.o \
	clc/data/StrUtil.o \
	clc/os/Clock.o \
	clc/os/Lock.o \
	clc/os/Monitor.o \
	clc/os/Thread.o \
	clc/storage/File.o \
	clc/storage/Path.o \
	clc/support/Debug.o \
	clc/support/Logger.o \
	ocher/device/Device.o \
	ocher/device/Filesystem.o \
	ocher/fmt/Layout.o \
	ocher/fmt/Meta.o \
	ocher/ocher.o \
	ocher/settings/Settings.o \
	ocher/ux/Browse.o \
	ocher/ux/Controller.o \
	ocher/ux/Pagination.o \
	ocher/ux/Renderer.o \
	ocher/ux/fb/BrowseFb.o \
	ocher/ux/fb/FactoryFb.o \
	ocher/ux/fb/RenderFb.o

ifeq ($(OCHER_AIRBAG_FD),1)
OCHER_OBJS += \
	airbag_fd/airbag_fd.o
endif

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

ifeq ($(OCHER_TARGET),kobo)
	OCHER_CFLAGS += \
		-DTARGET_KOBO \
		-DTARGET_KOBOTOUCH
endif

ifeq ($(OCHER_UI_SDL),1)
	OCHER_CFLAGS += -DOCHER_UI_SDL
	OCHER_OBJS += \
		ocher/input/SdlLoop.o \
		ocher/output/sdl/FbSdl.o \
		ocher/ux/fb/FactoryFbSdl.o
	OCHER_LIBS += -lSDL
endif

ifeq ($(OCHER_UI_MX50),1)
	OCHER_CFLAGS += -DOCHER_UI_MX50
	OCHER_OBJS += \
		ocher/output/mx50/fb.o \
		ocher/ux/fb/FactoryFbMx50.o
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
		clc/tui/Tui.o \
		ocher/ux/ncurses/Browse.o \
		ocher/ux/ncurses/RenderCurses.o \
		ocher/ux/ncurses/FactoryNC.o
	OCHER_LIBS += -lncurses -lform
endif

OCHER_OBJS += \
	ocher/output/FreeType.o

$(OCHER_OBJS): Makefile ocher.config $(BUILD_DIR)/ocher_config.h

$(BUILD_DIR)/ocher_config.h: Makefile ocher.config
CONFIG_BOOL=OCHER_DEV OCHER_DEBUG OCHER_AIRBAG_FD OCHER_EPUB OCHER_TEXT OCHER_HTML OCHER_UI_FD OCHER_UI_NCURSES OCHER_UI_SDL OCHER_UI_MX50
ocher_config_clean:
	@echo "CONFIG	ocher_config.h"
	@mkdir -p $(BUILD_DIR)
	@rm -f $(BUILD_DIR)/ocher_config.h
OCHER_%:
	@([ "$(OCHER_$*)" = "1" ] && echo "#define OCHER_$* 1" || echo "/* OCHER_$* */") >> $(BUILD_DIR)/ocher_config.h
$(BUILD_DIR)/ocher_config.h: ocher_config_clean $(CONFIG_BOOL)

#ODIR=obj
#OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

.c.o:
	$(MSG) "CC	$*.c"
	$(QUIET)$(CC) --std=c99 -c $(CFLAGS) $(OCHER_CFLAGS) $*.c -o $@

.cpp.o:
	$(MSG) "CXX	$*.cpp"
	$(QUIET)$(CXX) -c $(CFLAGS) $(OCHER_CFLAGS) $*.cpp -o $@

ocher: $(BUILD_DIR)/ocher
$(BUILD_DIR)/ocher: $(ZLIB_LIB) $(FREETYPE_LIB) $(MXML_LIB) $(OCHER_OBJS)
	$(MSG) "LINK	$@"
	$(QUIET)$(CXX) $(LD_FLAGS) $(CFLAGS) $(OCHER_CFLAGS) -o $@ $(OCHER_OBJS) $(ZLIB_LIB) $(FREETYPE_LIB) $(MXML_LIB) -ldl

clean: zlib_clean freetype_clean mxml_clean ocher_config_clean
	rm -f $(OCHER_OBJS) $(BUILD_DIR)/ocher

unittestpp:

ochertest:
	# TODO

test: unittestpp ocher ochertest
	# TODO

dist: ocher
	tar -C $(BUILD_DIR) -Jcf ocher-`uname -s`-$(OCHER_MAJOR).$(OCHER_MINOR).$(OCHER_PATCH).tar.xz ocher

dist-src: clean
	git status clc dl doc ocher
	tar -Jcf ocher-src-$(OCHER_MAJOR).$(OCHER_MINOR).$(OCHER_PATCH).tar.xz Makefile README clc dl doc ocher

doc:
	cd ocher && doxygen ../doc/Doxyfile

help:
	@echo "Edit ocher.config with your desired settings, then 'make'."
	@echo ""
	@echo "Targets:"
	@echo "	clean		Clean"
	@echo "	fonts		Download GPL fonts"
	@echo "*	ocher		Build the e-reader software"
	@echo "	ochertest	Build the unit tests"
	@echo "	test		Build and run the unit tests"
	@echo "	doc		Run Doxygen"
	@echo "	dist		Build distribution packages"

