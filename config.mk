VERSION=0.1.0
TIMESTAMP:=$(shell date "+%F %T%z")

# Client library SO version. Bump if incompatible API/ABI changes are made.
SOVERSION=1

UNAME:=$(shell uname -s)
CFLAGS?=-Wall -ggdb -O2 \
	-DDEBUG \
	-I../build_package/libftm/include \
	-I../build_package/mosquitto/usr/local/include \
	-I../build_package/libconfig-1.4.9/include \
	-I../build_package/libnxjson/include

LIB_CFLAGS:=${CFLAGS} ${CPPFLAGS} -I. 
LIB_CXXFLAGS:=$(LIB_CFLAGS) ${CPPFLAGS}
LIB_LDFLAGS:=${LDFLAGS}


LIB_LIBS:= -lftlm -lftm -lconfig -lnxjson -lmosquitto

FTLM_CFLAGS:=${CFLAGS} ${CPPFLAGS} -I../lib -DVERSION="\"${VERSION}\""
FTLM_LDFLAGS:=$(LDFLAGS) ${LIB_LIBS} \
		-L. \
		-L../build_package/libftm/lib \
		-L../build_package/libconfig-1.4.9/lib \
		-L../build_package/libnxjson/lib \
		-L../build_package/mosquitto/usr/local/lib

LIB_CFLAGS:=$(LIB_CFLAGS) -fPIC
LIB_CXXFLAGS:=$(LIB_CXXFLAGS) -fPIC

MAKE_ALL:=ftlm
ifeq ($(WITH_DOCS),yes)
	MAKE_ALL:=$(MAKE_ALL) docs
endif

CROSS_COMPILE=arm-openwrt-linux-uclibcgnueabi-
CC=gcc
INSTALL?=install
prefix=/home/xtra/work/cortina/build_package/ftlm
STRIP?=strip
