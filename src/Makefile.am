lib_LTLIBRARIES = libechoicebox.la
libechoicebox_la_SOURCES = echoicebox.c echoicebox_aux.c
include_HEADERS = echoicebox.h

AM_CFLAGS = -Wall

INCLUDES = @EFL_CFLAGS@
libechoicebox_la_LDFLAGS = @EFL_LIBS@ -no-undefined -version-info 2:0:2 --as-needed
