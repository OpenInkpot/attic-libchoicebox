lib_LTLIBRARIES = libechoicebox.la
libechoicebox_la_SOURCES = echoicebox.c echoicebox_aux.c
include_HEADERS = echoicebox.h

AM_CFLAGS = -Wall

INCLUDES = @EVAS_CFLAGS@ @ECORE_CFLAGS@ @ECORE_EVAS_CFLAGS@ @EDJE_CFLAGS@
libechoicebox_la_LDFLAGS = @EVAS_LIBS@ @ECORE_LIBS@ @ECORE_EVAS_LIBS@ @EDJE_LIBS@ -no-undefined -version-info 1:0:1 --as-needed
