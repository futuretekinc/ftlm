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


FTLM_LIBS:= -lftlm -lftm -lftlmapi -lconfig -lnxjson -lmosquitto -lssl -lcrypto -ldl 
FTLM_CONSOLE_LIBS:= -lftlm -lftm -lftlmapi -lconfig -lnxjson -lmosquitto -lssl -lcrypto -ldl 

FTLM_CFLAGS:=${CFLAGS} ${CPPFLAGS} \
		-I../lib \
		-I../build_package/libftm/include \
		-DVERSION="\"${VERSION}\""
FTLM_LDFLAGS:=$(LDFLAGS) ${FTLM_LIBS} \
		-L. \
		-L../build_package/libftm/lib \
		-L../build_package/libconfig-1.4.9/lib \
		-L../build_package/libnxjson/lib \
		-L../build_package/openssl/usr/openssl/lib \
		-L../build_package/mosquitto/usr/local/lib

FTLM_CONSOLE_LDFLAGS:=$(LDFLAGS) ${FTLM_CONSOLE_LIBS} \
		-L. \
		-L../build_package/libftm/lib \
		-L../build_package/libconfig-1.4.9/lib \
		-L../build_package/libnxjson/lib \
		-L../build_package/openssl/usr/openssl/lib \
		-L../build_package/mosquitto/usr/local/lib

LIB_CFLAGS:=$(LIB_CFLAGS) -fPIC
LIB_CXXFLAGS:=$(LIB_CXXFLAGS) -fPIC

MAKE_ALL:=ftlm
ifeq ($(WITH_DOCS),yes)
	MAKE_ALL:=$(MAKE_ALL) docs
endif

CROSS_COMPILE?=
CC=gcc
INSTALL?=install
prefix=$(CURDIR)/../build_package/ftlm
STRIP?=strip
